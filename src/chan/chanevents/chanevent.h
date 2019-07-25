#ifndef INCLUDED_CHAN_CHANEVENTS_CHANEVENT
#define INCLUDED_CHAN_CHANEVENTS_CHANEVENT

// TODO: explain it's a generic blah blah...

#include <chan/chanevents/chanprotocol.h>
#include <chan/chanevents/fulfillmentlockguard.h>
#include <chan/chanstate/chanstate.h>
#include <chan/debug/trace.h>
#include <chan/errors/error.h>
#include <chan/errors/errorcode.h>
#include <chan/errors/noexcept.h>
#include <chan/errors/uncaughtexceptions.h>
#include <chan/event/ioevent.h>
#include <chan/select/lasterror.h>
#include <chan/select/select.h>
#include <chan/threading/lockguard.h>

#include <algorithm>  // std::swap, std::find_if
#include <cassert>
#include <list>
#include <utility>  // std::move (if C++11)

namespace chan {

template <typename POLICY>
class ChanEvent {
  public:
    typedef typename POLICY::State    State;
    typedef typename POLICY::Teammate Teammate;
    typedef typename POLICY::Opponent Opponent;

  private:
    Teammate     me;
    Opponent     them;  // used when I'm a visitor (as opposed to a sitter)
    State&       chanState;
    mutable bool selectOnDestroy;

  public:
    ChanEvent(State& chanState, const Teammate& me);
    ChanEvent(const ChanEvent& other);
    ~ChanEvent() CHAN_THROWS;

    void    touch() CHAN_NOEXCEPT;
    IoEvent file(const EventContext&);
    IoEvent fulfill(IoEvent);
    void    cancel(IoEvent);

  private:
    IoEvent attemptTransfer();
    void    cleanup();
};

// Transferring an `OBJECT` between a sender and a receiver can be expressed as
// a non-member function template.  There is also a special overload for
// `void`, since `void` channels never transfer objects.
template <typename OBJECT>
void transfer(const ChanSender<OBJECT>&   sender,
              const ChanReceiver<OBJECT>& receiver) {
    assert(receiver.destination);

    if (sender.transferMode == ChanSender<OBJECT>::MOVE) {
        assert(sender.moveFrom);

#if __cplusplus >= 201103
        *receiver.destination = std::move(*sender.moveFrom);
#else
        using std::swap;
        swap(*receiver.destination, *sender.moveFrom);
#endif
    }
    else {
        assert(sender.transferMode == ChanSender<OBJECT>::COPY);
        assert(sender.copyFrom);

        *receiver.destination = *sender.copyFrom;
    }
}

// `Chan<void>` is special.  No transfer takes place on such channels.
inline void transfer(const ChanSender<void>&   sender,
                     const ChanReceiver<void>& receiver) {
    // By convention, `receiver.destination` is null, `sender.transferMode` is
    // `MOVE`, and `sender.moveFrom` is null.  There's nothing to do here.
    assert(!receiver.destination);
    assert(sender.transferMode == ChanSender<void>::MOVE);
    assert(!sender.moveFrom);

    // `-W-unused-parameter`
    (void)sender;
    (void)receiver;
}

template <typename OBJECT>
void transfer(const ChanReceiver<OBJECT>& receiver,
              const ChanSender<OBJECT>&   sender) {
    transfer(sender, receiver);
}

template <typename POLICY>
ChanEvent<POLICY>::ChanEvent(State& chanState, const Teammate& me)
: me(me)
, chanState(chanState)
, selectOnDestroy(true) {
}

template <typename POLICY>
ChanEvent<POLICY>::ChanEvent(const ChanEvent& other)
: me(other.me)
, them(other.them)
, chanState(other.chanState)
, selectOnDestroy(other.selectOnDestroy) {
    assert(selectOnDestroy);
    other.selectOnDestroy = false;
}

template <typename POLICY>
ChanEvent<POLICY>::~ChanEvent() CHAN_THROWS {
    if (selectOnDestroy && !uncaughtExceptions()) {
        if (select(*this)) {
            throw lastError();
        }
    }
}

template <typename POLICY>
void ChanEvent<POLICY>::touch() CHAN_NOEXCEPT {
    // I'm being called by `select`, so there's no need to call `select` in my
    // destructor.
    selectOnDestroy = false;
}

template <typename POLICY>
IoEvent ChanEvent<POLICY>::file(const EventContext& context) {
    // We don't get `EventContext` until `file` is called on us by `select`.
    me.context = context;

    // I'll be adding myself to the list of teammates, so first I need to
    // allocate pipe and a node for the list.
    me.pipe = chanState.pipePool.allocate();

    std::list<Teammate> oneNode;
    oneNode.push_back(me);

    // Now add the node to the list of teammates, and based on what I see in
    // the `ChanState`, begin the protocol as either a "sitter" or as a
    // "visitor."
    CHAN_WITH_LOCK(chanState.mutex) {
        std::list<Teammate>& teammates = POLICY::teammates(chanState);
        teammates.splice(teammates.end(), oneNode);

        const std::list<Opponent>& opponents = POLICY::opponents(chanState);
        if (teammates.size() == 1 && !opponents.empty()) {
            // I'm a visitor.  Write HI to the sitter and then the `IoEvent` is
            // to wait for the sitter to respond.
            CHAN_TRACE("I'm a visitor on channel ", &chanState);

            them = opponents.front();
            ++them.pipe->referenceCount;
        }
        else {
            // I'm a sitter.  The `IoEvent` is to wait for a visitor to contact
            // me.
            CHAN_TRACE("I'm a sitter on channel ", &chanState);

            IoEvent waitForVisitor;
            waitForVisitor.read = true;
            waitForVisitor.file = me.pipe->fromVisitor;
            return waitForVisitor;
        }
    }  // CHAN_WITH_LOCK(chanState.mutex)

    // If we made it here, then we're a visitor, not a sitter.  Acquire locks
    // in an attempt to exchange a value with `them`.
    return attemptTransfer();
}

template <typename POLICY>
IoEvent ChanEvent<POLICY>::attemptTransfer() {
    // `attemptTransfer` assumes that we're a visitor, so both `me` and
    // `them` must be valid.  One way to check this is to check the `Pipe*`
    // members.
    assert(me.pipe);
    assert(them.pipe);

    IoEvent event;
    event.read = true;
    event.file = me.pipe->fromVisitor;

    CHAN_TRACE("About to lock for visitation.  me.context=",
               me.context,
               " them.context=",
               them.context);

    FulfillmentLockGuard lock(me.context.fulfillment->mutex,
                              them.context.fulfillment->mutex);

    CHAN_TRACE("Got lock for visitation.  me.context=",
               me.context,
               " them.context=",
               them.context);

    if (me.context.fulfillment->state != SelectorFulfillment::FULFILLABLE) {
        // While I was acquiring `lock`, above, another event snuck in and
        // fulfilled one of the events in my `select` statement.  We're
        // done.

        // `select` won't act on the `IoEvent` we return, but return an
        // `IoEvent` as if we were becoming a sitter, so that when `cancel` is
        // called on us later, we can read from the appropriate file.
        CHAN_TRACE("Somebody fulfilled me while I was visiting in channel ",
                   &chanState);
        return event;
    }

    if (them.context.fulfillment->state != SelectorFulfillment::FULFILLABLE) {
        // Some event in their `select` statement has already been fulfilled,
        // or they're handling an error.
        // I will become a sitter, because if there's anyone after them I could
        // visit, they will get `POKE`d and visit me instead.
        //
        // TODO: Is this always correct, though?
        CHAN_TRACE("Somebody fulfilled them while I was visiting in channel ",
                   &chanState);
        return event;
    }

    // Neither their `select` nor my `select` has had an event fulfilled on
    // it, so I will attempt a value transfer.
    try {
        assert(me.context.fulfillment);
        assert(them.context.fulfillment);

        me.context.fulfillment->state = SelectorFulfillment::FULFILLED;
        me.context.fulfillment->fulfilledEventKey = me.context.eventKey;

        them.context.fulfillment->state = SelectorFulfillment::FULFILLED;
        them.context.fulfillment->fulfilledEventKey = them.context.eventKey;

        transfer(me, them);
    }
    catch (...) {
        // Notify the sitter that the transfer failed, and then rethrow
        // the exception.
        writeMessage(them.pipe->toSitter, ChanProtocolMessage::ERROR);
        lock.unlock();
        cleanup();
        throw;
    }

    // We did it!
    writeMessage(them.pipe->toSitter, ChanProtocolMessage::DONE);
    lock.unlock();
    cleanup();

    event.fulfilled = true;  // TODO: Do I need to set the fulfillment as well?
                             // The mutex is still locked and will remain so,
                             // so I don't see why I'd need to.
    return event;
}

template <typename POLICY>
IoEvent ChanEvent<POLICY>::fulfill(IoEvent event) {
    assert(event.read);
    assert(event.file == me.pipe->fromVisitor);

    // If we get poked and discover that a former `them`'s `Pipe`'s reference
    // count has gone to zero, we will deallocate the `Pipe` only _after_ we've
    // unlocked the `ChanState`'s mutex.  Hence this variable.
    Pipe* pipeToDeallocate = 0;

    // TODO: handle special/error cases (hangup, error, invalid)

    switch (const ChanProtocolMessage message = readMessage(event.file)) {
        case ChanProtocolMessage::DONE:
            // `fulfill` will never get called by `select` due to receiving a
            // `DONE` message, because if `::poll` wakes up and sees that
            // something was fulfilled, it won't handle the event.
            // However, the implementation of `cancel` might call `fulfill` if
            // it knows that we have been fulfilled, in order to handle either
            // the `DONE` or `ERROR` case.
            assert(me.context.fulfillment);
            assert(me.context.fulfillment->fulfilledEventKey ==
                   me.context.eventKey);
            CHAN_TRACE("About to call cleanup() after handling DONE in ",
                       &chanState);
            cleanup();
            return event;  // in this case, the return value doesn't matter
        case ChanProtocolMessage::ERROR:
            // An exception was thrown on the other end of the channel during
            // the transfer.  We don't know what exactly, but we can report
            // that something went wrong.  The thread on the other side of the
            // channel will throw the original exception.
            CHAN_TRACE("About to call cleanup() after handling ERROR in ",
                       &chanState);
            cleanup();
            throw Error(ErrorCode::TRANSFER);
        default: {
            assert(message == ChanProtocolMessage::POKE);
            CHAN_TRACE("Handling a POKE in ", &chanState);
            // We were waiting for a visitor to greet us, but instead
            // somebody informs us that we are one of two sitters waiting
            // for nobody, and so they poked us so that we will play the
            // visitor role with the other sitter. Of course, it could be
            // that by the time we get around to locking `chanState`,
            // things have changed.  So we might end up remaining a sitter
            // after all.
            LockGuard lock(chanState.mutex);

            std::list<Teammate>& teammates = POLICY::teammates(chanState);
            const std::list<Opponent>& opponents =
                POLICY::opponents(chanState);

            // Assert that we are at the front of `teammates`.  Checking
            // the `Pipe*` suffices.  Also check that we were poked.
            assert(!teammates.empty());
            assert(teammates.front().pipe == me.pipe);
            assert(teammates.front().isPoked);
            (void)teammates;  // unused variable when assertions are disabled

            teammates.front().isPoked = false;  // we're handling it now

            if (!opponents.empty() && !opponents.front().isPoked) {
                // There's an opponent we can visit.
                CHAN_TRACE("Found opponent while handling POKE in ",
                           &chanState);

                // I'm about to designate `them`.  If I've already done
                // this before, then now's the time to reduce the reference
                // count of the `pipe` of the previous `them`.  In either
                // case, I'll increase the reference count of the new
                // `them`.
                if (them.pipe && --them.pipe->referenceCount == 0) {
                    CHAN_TRACE("Decremented their pipe ",
                               them.pipe,
                               " down to ",
                               them.pipe->referenceCount);
                    pipeToDeallocate = them.pipe;
                }

                them = opponents.front();
                ++them.pipe->referenceCount;
            }
            else {
                // Either there's no one to visit, or the only candidate
                // has also been poked.  So, we remain a sitter (event
                // unchanged).
                CHAN_TRACE("No opponent or also poked while handling POKE in ",
                           &chanState);
                return event;
            }
        }
    }

    // If we've gotten to this point, then we were poked and found a sitter to
    // visit.
    event = attemptTransfer();
    CHAN_TRACE("Returned from attemptTransfer()");

    if (pipeToDeallocate) {
        chanState.pipePool.deallocate(pipeToDeallocate);
    }

    return event;
}

template <typename POLICY>
void ChanEvent<POLICY>::cancel(IoEvent ioEvent) {
    // If I'm the fulfilled one, I'm guaranteed to have been sent a
    // message.  I can check it now to determine whether it's an `ERROR`.
    // Otherwise, just `cleanup`.
    if (me.context.fulfillment->state == SelectorFulfillment::FULFILLED &&
        me.context.fulfillment->fulfilledEventKey == me.context.eventKey) {
        // TODO: I found in my debugging that `POKE` is possible here.  How?
        CHAN_TRACE("About to read from cancel because I was the one "
                   "fulfilled.  I'm on channel ",
                   &chanState);
        const ChanProtocolMessage message = readMessage(ioEvent.file);
        cleanup();
        if (message == ChanProtocolMessage::ERROR) {
            throw Error(ErrorCode::TRANSFER);
        }
    }
    else {
        CHAN_TRACE("About to call cleanup from cancel because I was not the "
                   "one fulfilled.  I'm on channel ",
                   &chanState);
        cleanup();
    }
}

// This function-like object is meant to be used to find a `ChanSender` or a
// `ChanReceiver` based on the value of its `Pipe*` member.
class PipeEquals {
    const Pipe* lookingFor;

  public:
    explicit PipeEquals(const Pipe* lookingFor)
    : lookingFor(lookingFor) {
    }

    bool operator()(const ChanParticipant& participant) const {
        return participant.pipe == lookingFor;
    }
};

template <typename POLICY>
void ChanEvent<POLICY>::cleanup() {
    // Depending on the reference counts that we see after locking the
    // mutex, we might afterward deallocate some pipe.  These flags keep
    // track of whether to do so.
    bool deallocateMyPipe    = false;
    bool deallocateTheirPipe = false;

    // To prevent memory deallocations from happening in the critical section
    // below, this `list` is used as a placeholder into which the node we
    // wish to remove from `teammates` can be spliced.
    std::list<Teammate> removedNode;

    CHAN_TRACE("In cleanup(), about to acquire mutex for chanState ",
               &chanState);
    CHAN_WITH_LOCK(chanState.mutex) {
        CHAN_TRACE("In cleanup(), acquired mutex for chanState ", &chanState);
        std::list<Teammate>& teammates = POLICY::teammates(chanState);

        // Find our entry in `teammates`.  Checking the `Pipe*` suffices.
        assert(!teammates.empty());

        const typename std::list<Teammate>::iterator found = std::find_if(
            teammates.begin(), teammates.end(), PipeEquals(me.pipe));

        assert(found != teammates.end());

        const bool weWereUpFront = found == teammates.begin();
        if (weWereUpFront) {
            CHAN_TRACE("Removing myself from the front of ", &chanState);
        }
        else {
            CHAN_TRACE("Removing myself from the not-front of ", &chanState);
        }

        // Remove me from `teammates`.
        removedNode.splice(removedNode.begin(), teammates, found);

        // Decrement reference count on visible `Pipe`s.
        deallocateMyPipe = --me.pipe->referenceCount == 0;
        CHAN_TRACE("Decremented my pipe ",
                   me.pipe,
                   " down to ",
                   me.pipe->referenceCount);

        if (them.pipe) {
            deallocateTheirPipe = --them.pipe->referenceCount == 0;
            CHAN_TRACE("Decremented their pipe ",
                       them.pipe,
                       " down to ",
                       them.pipe->referenceCount);
        }

        // If we need to poke the next sitter, do so.
        std::list<Opponent>& opponents = POLICY::opponents(chanState);
        if (weWereUpFront && !teammates.empty() && !opponents.empty()) {
            // There was somebody behind us in `teammates` that could be
            // talking the the first of the `opponents`.  Poke the
            // teammate.
            Teammate& nextUp = teammates.front();
            nextUp.isPoked   = true;
            writeMessage(nextUp.pipe->toSitter, ChanProtocolMessage::POKE);
        }
    }

    if (deallocateMyPipe) {
        chanState.pipePool.deallocate(me.pipe);
    }

    if (deallocateTheirPipe) {
        chanState.pipePool.deallocate(them.pipe);
    }
}

}  // namespace chan

#endif
