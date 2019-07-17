#ifndef INCLUDED_CHAN_TIMEEVENTS_DEADLINE
#define INCLUDED_CHAN_TIMEEVENTS_DEADLINE

#include <chan/event/ioevent.h>
#include <chan/time/duration.h>
#include <chan/time/timepoint.h>

#include <algorithm>
#if __cplusplus >= 201103
#include <chrono>
#endif

namespace chan {

class DeadlineEvent {
    TimePoint when;

  public:
    explicit DeadlineEvent(TimePoint when)
    : when(when) {
    }

    IoEvent file() const {
        IoEvent result;
        result.timeout    = true;
        result.expiration = when;
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

inline DeadlineEvent deadline(TimePoint when) {
    return DeadlineEvent(when);
}

#if __cplusplus >= 201103
inline DeadlineEvent deadline(std::chrono::steady_clock::time_point when) {
    typedef std::chrono::steady_clock steady_clock;

    // We use millisecond precision here.  This is fine, since `::poll()` can
    // do no better.
    const steady_clock::duration duration = when - steady_clock::now();
    const long durationMs = duration / std::chrono::milliseconds(1);

    return DeadlineEvent(now() + milliseconds(durationMs));
}
#endif

}  // namespace chan

#endif
