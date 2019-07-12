#ifndef INCLUDED_CHAI_IOEVENT
#define INCLUDED_CHAI_IOEVENT

namespace chai {

struct IoEvent {
    enum {
        READ      = 1,
        WRITE     = 1 << 1,
        HANGUP    = 1 << 2,
        TIMEOUT   = 1 << 3,
        FULFILLED = 1 << 4
    };

    int event;  // bitfield of above flags

  public:
    union {
        int file;          // descriptor, for file-related events
        int milliseconds;  // interval, for timeout event
    };
};

}  // namespace chai

#endif
