#ifndef INCLUDED_CHAN_CHANEVENTS_SENDEVENT
#define INCLUDED_CHAN_CHANEVENTS_SENDEVENT

#include <chan/chanevents/chanevent.h>
#include <chan/chanstate/chanstate.h>

#include <list>

namespace chan {

// `SendEvent` and `RecvEvent` share a lot of logic.  The shared code is the
// `ChanEvent` class template.  The differences between `SendEvent` and
// `RecvEvent` are encoded in the `POLICY` type argument to `ChanEvent`.
// `SendEventPolicy`, below, specifies the sender-specific behavior needed for
// `SendEvent`.
template <typename OBJECT>
struct SendEventPolicy {
    typedef ChanState<OBJECT>    State;
    typedef ChanSender<OBJECT>   Teammate;
    typedef ChanReceiver<OBJECT> Opponent;

    // If you're a sender, then your "teammates" are senders.
    static std::list<Teammate>& teammates(State& state) {
        return state.senders;
    }

    // If you're a sender, then your "opponents" are receivers.
    static std::list<Opponent>& opponents(State& state) {
        return state.receivers;
    }
};

template <typename OBJECT>
class SendEvent : public ChanEvent<SendEventPolicy<OBJECT> > {
    typedef ChanEvent<SendEventPolicy<OBJECT> > Base;

  public:
    SendEvent(ChanState<OBJECT>& chanState, OBJECT* source);
    SendEvent(ChanState<OBJECT>& chanState, const OBJECT* source);
};

template <typename OBJECT>
ChanSender<OBJECT> makeSender(OBJECT* source) {
    ChanSender<OBJECT> sender;

    sender.transferMode = ChanSender<OBJECT>::MOVE;
    sender.moveFrom     = source;

    return sender;
}

template <typename OBJECT>
ChanSender<OBJECT> makeSender(const OBJECT* source) {
    ChanSender<OBJECT> sender;

    sender.transferMode = ChanSender<OBJECT>::COPY;
    sender.copyFrom     = source;

    return sender;
}

template <typename OBJECT>
SendEvent<OBJECT>::SendEvent(ChanState<OBJECT>& chanState, OBJECT* source)
: Base(chanState, makeSender(source)) {
}

template <typename OBJECT>
SendEvent<OBJECT>::SendEvent(ChanState<OBJECT>& chanState,
                             const OBJECT*      source)
: Base(chanState, makeSender(source)) {
}

}  // namespace chan

#endif
