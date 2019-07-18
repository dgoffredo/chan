#ifndef INCLUDED_CHAN_CHANEVENTS_RECVEVENT
#define INCLUDED_CHAN_CHANEVENTS_RECVEVENT

#include <chan/chanevents/chanevent.h>
#include <chan/chanstate/chanstate.h>

#include <list>

namespace chan {

// `SendEvent` and `RecvEvent` share a lot of logic.  The shared code is the
// `ChanEvent` class template.  The differences between `SendEvent` and
// `RecvEvent` are encoded in the `POLICY` type argument to `ChanEvent`.
// `RecvEventPolicy`, below, specifies the receiver-specific behavior needed
// for `RecvEvent`.
template <typename OBJECT>
struct RecvEventPolicy {
    typedef ChanState<OBJECT>    State;
    typedef ChanReceiver<OBJECT> Teammate;
    typedef ChanSender<OBJECT>   Opponent;

    // If you're a receiver, then your "teammates" are receivers.
    static std::list<Teammate>& teammates(State& state) {
        return state.receivers;
    }

    // If you're a receiver, then your "opponents" are senders.
    static std::list<Opponent>& opponents(State& state) {
        return state.senders;
    }
};

template <typename OBJECT>
class RecvEvent : public ChanEvent<RecvEventPolicy<OBJECT> > {
    typedef ChanEvent<RecvEventPolicy<OBJECT> > Base;

  public:
    RecvEvent(ChanState<OBJECT>& chanState, OBJECT* destination);
};

template <typename OBJECT>
ChanReceiver<OBJECT> makeReceiver(OBJECT* destination) {
    assert(destination);

    ChanReceiver<OBJECT> receiver;
    receiver.destination = destination;
    return receiver;
}

template <typename OBJECT>
RecvEvent<OBJECT>::RecvEvent(ChanState<OBJECT>& chanState, OBJECT* destination)
: Base(chanState, makeReceiver(destination)) {
}

}  // namespace chan

#endif
