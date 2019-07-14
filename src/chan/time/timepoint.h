#ifndef INCLUDED_CHAN_TIME_TIMEPOINT
#define INCLUDED_CHAN_TIME_TIMEPOINT

#include <chan/time/duration.h>

#include <cstdlib>
#include <ctime>
#include <limits>

namespace chan {

class TimePoint {
    std::time_t seconds;
    long        milliseconds;

    enum {
        TIME_T__MUST__BE__SIGNED =
            1 / std::numeric_limits<std::time_t>::is_signed
    };

    friend Duration operator-(TimePoint left, TimePoint right) {
        return chan::milliseconds(long(left.seconds - right.seconds) * 1000 +
                                  (left.milliseconds - right.milliseconds));
    }

    friend TimePoint now();

  public:
    // A default constructed `TimePoint` represents an unspecified point of
    // time in the past, no earlier than when the current system last booted
    // (it's the zero point of some monotonic steady clock).
    TimePoint()
    : seconds()
    , milliseconds() {
    }

    TimePoint& operator+=(Duration duration) {
        // Get the (quotient, remainder) of dividing the number of milliseconds
        // in `duration` by 1000, thus getting number of seconds and remaining
        // milliseconds.
        const std::ldiv_t divided =
            std::ldiv(duration / chan::milliseconds(1), 1000);

        seconds += divided.quot;
        milliseconds += divided.rem;

        if (milliseconds < 0) {
            --seconds;
            milliseconds += 1000;
        }
        else if (milliseconds >= 1000) {
            ++seconds;
            milliseconds -= 1000;
        }

        return *this;
    }

    TimePoint& operator-=(Duration duration) {
        return *this += -duration;
    }
};

// Return the current time according to some monotonic steady clock.  Note that
// values returned by this function are intended to be used to calculate
// deadlines and timeouts for use with `chan::select`.
TimePoint now();

inline TimePoint operator+(TimePoint point, Duration duration) {
    return point += duration;
}

inline TimePoint operator+(Duration duration, TimePoint point) {
    return point + duration;  // calls the other `operator+`
}

inline TimePoint operator-(TimePoint point, Duration duration) {
    return point -= duration;
}

#define CHAN_DEFINE_COMPARISON(OP)                             \
    inline bool operator OP(TimePoint left, TimePoint right) { \
        return left - right OP Duration();                     \
    }

CHAN_DEFINE_COMPARISON(==)
CHAN_DEFINE_COMPARISON(!=)
CHAN_DEFINE_COMPARISON(<)
CHAN_DEFINE_COMPARISON(<=)
CHAN_DEFINE_COMPARISON(>)
CHAN_DEFINE_COMPARISON(>=)

#undef CHAN_DEFINE_COMPARISON

}  // namespace chan

#endif
