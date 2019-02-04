#ifndef INCLUDED_CHAN_EVENT_IOEVENT
#define INCLUDED_CHAN_EVENT_IOEVENT

// `IoEvent` is the value type used by objects satisying the _Event_ concept to
// communicate with `select`.  See this package's `README.md` file for an
// explanation of each of `IoEvent`'s valid states.

namespace chan {

struct IoEvent {
    bool read      : 1;  // readability on `this->file`
    bool write     : 1;  // writability on `this->file`
    bool hangup    : 1;  // the other end of `this->file` was closed
    bool timeout   : 1;  // after `this->milliseconds` have elapsed
    bool fulfilled : 1;  // successful fulfillment returned from `fulfill(...)`

    union {
        int file;          // descriptor, for file-related events
        int milliseconds;  // interval, for timeout event
    };

    IoEvent();
};

inline IoEvent::IoEvent()
: read(), write(), hangup(), timeout(), fulfilled()
{}

}  // namespace chan

#endif
