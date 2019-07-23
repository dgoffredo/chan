#ifndef INCLUDED_CHAN_CHANEVENTS_CHANEVENT
#define INCLUDED_CHAN_CHANEVENTS_CHANEVENT

// TODO: explain it's a generic blah blah...

#include <chan/chanevents/chanprotocol.h>
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

#include <algorithm>  // std::swap
#include <cassert>
#include <list>
#include <utility>  // std::move (if C++11)

namespace chan {

class EventContext;

template <typename POLICY>
class ChanEvent {
  public:
    typedef typename POLICY::State    State;
    typedef typename POLICY::Teammate Teammate;
    typedef typename POLICY::Opponent Opponent;

  private:
    enum Role { SITTER, VISITOR };

    Role         role;
    Teammate     me;
    Opponent     them;  // used when my `role` is `VISITOR`
    State&       chanState;
    mutable bool selectOnDestroy;

  public:
    ChanEvent(State& chanState, const Teammate& me);
    ChanEvent(const ChanEvent& other);
    ~ChanEvent() CHAN_THROWS;

    IoEvent file(const EventContext&);
    IoEvent fulfill(IoEvent);
    void    cancel(IoEvent);

  private:
    IoEvent fulfillSitter(IoEvent);
    IoEvent fulfillVisitor(IoEvent);
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
, them()
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
IoEvent ChanEvent<POLICY>::file(const EventContext&) {
    // I'm being called by `select`, so there's no need to call `select` in my
    // destructor.
    selectOnDestroy = false;

    // I'll be adding myself to the list of teammates, so first I need to
    // allocate pipes and a node for the list.
    me.pipes = chanState.pipePool.allocate();

    std::list<Teammate> oneNode;
    oneNode.push_back(me);

    // Now add the node to the list of teammates, and based on what I see in
    // the `ChanState`, begin the protocol as either a "sitter" or as a
    // "visitor."
    LockGuard lock(chanState.mutex);

    std::list<Teammate>& teammates = POLICY::teammates(chanState);
    teammates.splice(teammates.end(), oneNode);

    const std::list<Opponent>& opponents = POLICY::opponents(chanState);
    if (teammates.size() == 1 && !opponents.empty()) {
        // I'm a visitor.  Write HI to the sitter and then the `IoEvent` is to
        // wait for the sitter to respond.
        role = VISITOR;
        CHAN_TRACE("I'm a visitor.");

        them = opponents.front();
        ++them.pipes->referenceCount;

        writeMessage(them.pipes->toSitter, ChanProtocolMessage::HI);

        IoEvent result;
        result.read = true;
        result.file = them.pipes->fromSitter;
        return result;
    }
    else {
        // I'm a sitter.  The `IoEvent` is to wait for a visitor to contact me.
        role = SITTER;
        CHAN_TRACE("I'm a sitter.");

        IoEvent result;
        result.read = true;
        result.file = me.pipes->fromVisitor;
        return result;
    }
}

template <typename POLICY>
IoEvent ChanEvent<POLICY>::fulfill(IoEvent event) {
    if (role == SITTER) {
        return fulfillSitter(event);
    }
    else {
        assert(role == VISITOR);
        return fulfillVisitor(event);
    }
}

template <typename POLICY>
void ChanEvent<POLICY>::cancel(IoEvent) {
    // The only difference that `role` makes in `cancel` is to whom I send a
    // `CANCEL` message.

    int pipe;
    if (role == SITTER) {
        pipe = me.pipes->toVisitor;
    }
    else {
        assert(role == VISITOR);
        assert(them.pipes);

        pipe = them.pipes->toSitter;
    }

    writeMessage(pipe, ChanProtocolMessage::CANCEL);
    cleanup();
}

template <typename POLICY>
IoEvent ChanEvent<POLICY>::fulfillSitter(IoEvent event) {
    assert(role == SITTER);
    assert(event.read);
    assert(event.file == me.pipes->fromVisitor);

    bool deallocateTheirPipes = false;

    // TODO: handle special/error cases (hangup, error, invalid)

    switch (const ChanProtocolMessage message = readMessage(event.file)) {
        case ChanProtocolMessage::HI: {
            // We're being visited.  Send a `READY` message to the visitor, and
            // then do a blocking read to get the response.  We have to block
            // because the visitor might start transferring the `OBJECT` as
            // soon as it reads our `READY`, and so we must wait to know what
            // happened.  If we didn't wait, then we might return from this
            // call, the `select` that's driving us could fulfill a different
            // event, and then execution could continue after that `select`
            // returns, possibly destroying the object from which the visitor
            // is trying to copy/move.
            // Note that as a consequence of this, no single call to `select`
            // can both send and receive on the same channel.  This next call
            // to `readMessage` would block forever.
            writeMessage(me.pipes->toVisitor, ChanProtocolMessage::READY);
            const ChanProtocolMessage reply =
                readMessage(me.pipes->fromVisitor);

            switch (reply) {
                case ChanProtocolMessage::DONE:
                    // We did it!
                    cleanup();
                    event.fulfilled = true;
                    return event;
                case ChanProtocolMessage::ERROR:
                    // An exception was thrown on the other end of the channel
                    // during the transfer.  We don't know what exactly, but we
                    // can report that something went wrong.  The thread on the
                    // other side of the channel will throw the original
                    // exception.
                    cleanup();
                    throw Error(ErrorCode::TRANSFER);
                default:
                    assert(reply == ChanProtocolMessage::CANCEL);
                    // The visitor cancelled.  We go back to waiting for
                    // another visitor.
                    return event;
            }
        }
        default: {
            assert(message == ChanProtocolMessage::POKE);
            // We were waiting for a visitor to greet us, but instead somebody
            // informs us that we are one of two sitters waiting for nobody,
            // and so they poked us so that we will play the visitor role with
            // the other sitter.
            // Of course, it could be that by the time we get around to locking
            // `chanState`, things have changed.  So we might end up remaining
            // a sitter after all.
            LockGuard lock(chanState.mutex);

            std::list<Teammate>& teammates = POLICY::teammates(chanState);
            const std::list<Opponent>& opponents =
                POLICY::opponents(chanState);

            // Assert that we are at the front of `teammates`.  Checking one of
            // the file descriptors in the `PipePair` suffices.  Also check
            // that we were poked.
            assert(!teammates.empty());
            assert(teammates.front().pipes->toVisitor == me.pipes->toVisitor);
            assert(teammates.front().isPoked);
            (void)teammates;  // unused variable when assertions are disabled

            teammates.front().isPoked = false;  // we're handling it now

            if (!opponents.empty() && !opponents.front().isPoked) {
                // There's an opponent we can visit.  Write `HI` to the sitter
                // and then the `IoEvent` is to wait for the sitter to respond.
                role = VISITOR;

                // I'm about to designate `them`.  If I've already done this
                // before, then now's the time to reduce the reference count
                // of the `pipes` of the previous `them`.  In either case, I'll
                // increase the reference count of the new `them`.
                if (them.pipes && --them.pipes->referenceCount == 0) {
                    CHAN_TRACE("Decremented their pipes ",
                               them.pipes,
                               " down to ",
                               them.pipes->referenceCount);
                    deallocateTheirPipes = true;
                }

                them = opponents.front();
                ++them.pipes->referenceCount;
                CHAN_TRACE("After poke, writing HI to ", them.pipes);
                writeMessage(them.pipes->toSitter, ChanProtocolMessage::HI);

                event.file = them.pipes->fromSitter;
            }
            else {
                // Either there's no one to visit, or the only candidate has
                // also been poked.  So, we remain a sitter (event unchanged).
            }

            if (deallocateTheirPipes) {
                chanState.pipePool.deallocate(them.pipes);
            }

            return event;
        }
    }
}

template <typename POLICY>
IoEvent ChanEvent<POLICY>::fulfillVisitor(IoEvent event) {
    assert(role == VISITOR);
    assert(event.read);
    assert(event.file == them.pipes->fromSitter);

    // TODO: handle special/error cases (hangup, error, invalid)

    switch (const ChanProtocolMessage message = readMessage(event.file)) {
        case ChanProtocolMessage::READY: {
            try {
                transfer(me, them);
            }
            catch (...) {
                // Notify the sitter that the transfer failed, and then rethrow
                // the exception.
                writeMessage(them.pipes->toSitter, ChanProtocolMessage::ERROR);
                cleanup();
                throw;
            }

            // We did it!
            writeMessage(them.pipes->toSitter, ChanProtocolMessage::DONE);
            cleanup();
            event.fulfilled = true;
            return event;
        }
        default:
            assert(message == ChanProtocolMessage::CANCEL);
            // Now we become a sitter.  Either there is nobody else to transfer
            // with, or the departing sitting opponent poked its next in line,
            // which will then send us `HI`.
            role = SITTER;
            // TODO: Is this write to `them.pipes->referenceCount` safe?
            if (--them.pipes->referenceCount == 0) {
                CHAN_TRACE("Decremented their pipes ",
                           them.pipes,
                           " down to ",
                           them.pipes->referenceCount);
                LockGuard lock(chanState.mutex);
                chanState.pipePool.deallocate(them.pipes);
                them = Opponent();
            }

            event.file = me.pipes->fromVisitor;
            return event;
    }
}

template <typename POLICY>
void ChanEvent<POLICY>::cleanup() {
    // Depending on the reference counts that we see after locking the mutex,
    // we might afterward deallocate some pipes.  These flags keep track of
    // whether to do so.
    bool deallocateMyPipes    = false;
    bool deallocateTheirPipes = false;

    // To prevent deallocations from happening in the critical section below,
    // this `list` is used as a placeholder into which the node we wish to
    // remove from `teammates` can be spliced.
    std::list<Teammate> removedNode;

    CHAN_WITH_LOCK(chanState.mutex) {
        std::list<Teammate>& teammates = POLICY::teammates(chanState);

        // Assert that we are at the front of `teammates`.  Checking one of the
        // file descriptors in the `PipePair` suffices.
        assert(!teammates.empty());
        assert(teammates.front().pipes->toVisitor == me.pipes->toVisitor);

        // Remove me from the (front) of `teammates`.
        removedNode.splice(removedNode.begin(), teammates, teammates.begin());

        // Decrement reference count on visible `PipePair`s.
        deallocateMyPipes = --me.pipes->referenceCount == 0;
        CHAN_TRACE("Decremented my pipes ",
                   me.pipes,
                   " down to ",
                   me.pipes->referenceCount);
        if (them.pipes) {
            deallocateTheirPipes = --them.pipes->referenceCount == 0;
            CHAN_TRACE("Decremented their pipes ",
                       them.pipes,
                       " down to ",
                       them.pipes->referenceCount);
        }

        // TODO: explain
        writeMessage(me.pipes->toVisitor, ChanProtocolMessage::CANCEL);

        // If we need to poke the next sitter, do so.
        std::list<Opponent>& opponents = POLICY::opponents(chanState);
        if (!teammates.empty() && !opponents.empty()) {
            // There was somebody behind us in `teammates` that could be
            // talking the the first of the `opponents`.  Poke the teammate.
            teammates.front().isPoked = true;

            writeMessage(teammates.front().pipes->toSitter,
                         ChanProtocolMessage::POKE);
        }
    }

    if (deallocateMyPipes) {
        chanState.pipePool.deallocate(me.pipes);
    }

    if (deallocateTheirPipes) {
        chanState.pipePool.deallocate(them.pipes);
    }
}

}  // namespace chan

#endif
