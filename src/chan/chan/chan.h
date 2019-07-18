#ifndef INCLUDED_CHAN_CHAN_CHAN
#define INCLUDED_CHAN_CHAN_CHAN

#include <chan/chanevents/recvevent.h>
#include <chan/chanevents/sendevent.h>
#include <chan/select/lasterror.h>
#include <chan/select/select.h>
#include <chan/threading/sharedptr.h>

namespace chan {

template <typename OBJECT>
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
: state(new ChanState<OBJECT>) {
}

template <typename OBJECT>
SendEvent<OBJECT> Chan<OBJECT>::send(const OBJECT& copyFrom) {
    return SendEvent<OBJECT>(*state, copyFrom);
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

}  // namespace chan

#endif
