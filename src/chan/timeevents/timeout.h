#ifndef INCLUDED_CHAN_TIMEEVENTS_TIMEOUT
#define INCLUDED_CHAN_TIMEEVENTS_TIMEOUT

#include <chan/event/ioevent.h>
#include <chan/time/duration.h>
#include <chan/time/timepoint.h>

#if __cplusplus >= 201103
#include <chrono>
#endif

namespace chan {

class TimeoutEvent {
    Duration duration;

  public:
    explicit TimeoutEvent(Duration duration)
    : duration(duration) {
    }

    IoEvent file() const {
        IoEvent result;
        result.timeout    = true;
        result.expiration = now() + duration;
        return result;
    }

    IoEvent fulfill(IoEvent) const {
        IoEvent result;
        result.fulfilled = true;
        return result;
    }

    void cancel(IoEvent) const {
    }
};

inline TimeoutEvent timeout(Duration duration) {
    return TimeoutEvent(duration);
}

#if __cplusplus >= 201103
inline TimeoutEvent timeout(std::chrono::milliseconds duration) {
    return TimeoutEvent(milliseconds(duration / std::chrono::milliseconds(1)));
}
#endif

}  // namespace chan

#endif
