#ifndef INCLUDED_CHAN_EVENT_IOEVENT
#define INCLUDED_CHAN_EVENT_IOEVENT

#include <chan/macros/macros.h>
#include <chan/time/timepoint.h>

#include <ostream>

// `IoEvent` is the value type used by objects satisying the _Event_ concept to
// communicate with `select`.  See this package's `README.md` file for an
// explanation of each of `IoEvent`'s valid states.

namespace chan {

struct IoEvent {
    bool read : 1;       // readability on `this->file`
    bool write : 1;      // writability on `this->file`
    bool timeout : 1;    // once `this->expiration` has passed
    bool fulfilled : 1;  // successful fulfillment returned from `fulfill(...)`
    bool hangup : 1;     // the other end of `this->file` was closed (maybe)
    bool error : 1;      // an error occurred on `this->file`
    bool invalid : 1;    // `this->file` is not a usable file descriptor

    int       file;        // descriptor, for file-related events
    TimePoint expiration;  // when to expire, for deadline event

    IoEvent();
};

inline IoEvent::IoEvent()
: read()
, write()
, timeout()
, fulfilled()
, hangup()
, error()
, invalid()
, file() {
}

inline std::ostream& operator<<(std::ostream& stream, IoEvent event) {
    if (event.fulfilled) {
        return stream << "[fulfilled]";
    }
    else if (event.timeout) {
        // The following `reinterpret_cast` is valid, but breaks encapsulation.
        // This is fine for debugging.
        return stream << "[timeout expiration=("
                      << reinterpret_cast<TimeSpec&>(event.expiration) << ")]";
    }
    else {
        stream << "[file=" << event.file;
#define MAYBE_PRINT_FLAG(NAME)          \
    if (event.NAME) {                   \
        stream << " " CHAN_QUOTE(NAME); \
    }
        CHAN_MAPP(MAYBE_PRINT_FLAG, (read, write, hangup, error, invalid))
#undef MAYBE_PRINT_FLAG

        return stream << "]";
    }
}

}  // namespace chan

#endif
