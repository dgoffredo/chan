#ifndef INCLUDED_CHAN_CHANSTATE_CHANSTATE
#define INCLUDED_CHAN_CHANSTATE_CHANSTATE

#include <chan/event/eventcontext.h>
#include <chan/files/pipe.h>
#include <chan/files/pipepool.h>

#include <list>

namespace chan {

// `ChanParticipant` are the fields common to `ChanSender` and `SendReceiver`.
struct ChanParticipant {
    Pipe*        pipe;
    EventContext context;

    // We were poked but have yet to respond to it.  This flag is used to avoid
    // a deadlock that would result from both a sender and a receiver being
    // poked before either had time to handle it -- then both would try to
    // visit the other.  Checking `isPoked` prevents that case.
    bool isPoked;

    ChanParticipant()
    : pipe()
    , context()
    , isPoked() {
    }
};

template <typename OBJECT>
struct ChanSender : public ChanParticipant {
    // In C++98, using the `MOVE` `TransferMode` performs a swap instead of a
    // move.  The real difference between the two values of this `enum` is
    // which of the members of the union below will be accessed.
    enum TransferMode { MOVE, COPY } transferMode;

    union {
        OBJECT*       moveFrom;
        const OBJECT* copyFrom;
    };
};

template <typename OBJECT>
struct ChanReceiver : public ChanParticipant {
    OBJECT* destination;
};

template <typename OBJECT>
struct ChanState {
    Mutex                            mutex;
    std::list<ChanSender<OBJECT> >   senders;
    std::list<ChanReceiver<OBJECT> > receivers;

    // `PipePool` manages concurrent access using its own `Mutex`, so I put it
    // apart from the other data members.
    PipePool pipePool;
};

}  // namespace chan

#endif
