#ifndef INCLUDED_CHAN_CHAN_CHAN
#define INCLUDED_CHAN_CHAN_CHAN

#include <chan/chanevents/recvevent.h>
#include <chan/chanevents/sendevent.h>
#include <chan/select/lasterror.h>
#include <chan/select/select.h>
#include <chan/threading/sharedptr.h>

namespace chan {

template <typename OBJECT = void>
class Chan {
    SharedPtr<ChanState<OBJECT> > state;

  public:
    Chan();

    SendEvent<OBJECT> send(const OBJECT& copyFrom);
    SendEvent<OBJECT> send(OBJECT* moveFrom);

    RecvEvent<OBJECT> recv(OBJECT* destination);
    OBJECT            recv();
};

template <typename OBJECT>
Chan<OBJECT>::Chan()
: state(new ChanState<OBJECT>()) {
}

template <typename OBJECT>
SendEvent<OBJECT> Chan<OBJECT>::send(const OBJECT& copyFrom) {
    return SendEvent<OBJECT>(*state, &copyFrom);
}

template <typename OBJECT>
SendEvent<OBJECT> Chan<OBJECT>::send(OBJECT* moveFrom) {
    return SendEvent<OBJECT>(*state, moveFrom);
}

template <typename OBJECT>
RecvEvent<OBJECT> Chan<OBJECT>::recv(OBJECT* destination) {
    return RecvEvent<OBJECT>(*state, destination);
}

template <typename OBJECT>
OBJECT Chan<OBJECT>::recv() {
    OBJECT result;
    switch (select(this->recv(&result))) {
        case 0:
            return result;
        default:
            throw lastError();
    }
}

// `class Chan` is specialized for `void`, the default type.  `Chan<void>` is
// a channel that does not transfer any values, but still participates in all
// of the synchronization.  It's convenient for when a "dummy" type is needed,
// e.g. for a "done" channel.  Rather than defining an empty `struct` for this
// purpose, or using something like `bool`, instead `Chan<>` can be used so
// that no type argument is needed.
template <>
class Chan<void> {
    SharedPtr<ChanState<void> > state;

  public:
    Chan();

    SendEvent<void> send();
    RecvEvent<void> recv();
};

inline Chan<void>::Chan()
: state(new ChanState<void>()) {
}

inline SendEvent<void> Chan<void>::send() {
    void* const null = 0;
    return SendEvent<void>(*state, null);
}

inline RecvEvent<void> Chan<void>::recv() {
    return RecvEvent<void>(*state, 0);
}

}  // namespace chan

#endif
