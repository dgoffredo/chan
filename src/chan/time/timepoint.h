#ifndef INCLUDED_CHAN_TIME_TIMEPOINT
#define INCLUDED_CHAN_TIME_TIMEPOINT

#include <chan/time/duration.h>

namespace chan {

class TimePoint {
    Duration offset;

    friend Duration operator-(TimePoint left, TimePoint right) {
        return left.offset - right.offset;
    }

#define CHAN_DEFINE_COMPARISON(OP)                             \
    friend bool operator OP(TimePoint left, TimePoint right) { \
        return left.offset OP right.offset;                    \
    }

    CHAN_DEFINE_COMPARISON(==)
    CHAN_DEFINE_COMPARISON(!=)
    CHAN_DEFINE_COMPARISON(<)
    CHAN_DEFINE_COMPARISON(<=)
    CHAN_DEFINE_COMPARISON(>)
    CHAN_DEFINE_COMPARISON(>=)

#undef CHAN_DEFINE_COMPARISON

    friend TimePoint now();

  public:
    // A default constructed `TimePoint` represents an unspecified point of
    // time in the past, no earlier than when the current system last booted
    // (it's the zero point of some monotonic steady clock).
    TimePoint() {
    }

    TimePoint& operator+=(Duration duration) {
        offset += duration;
        return *this;
    }

    TimePoint& operator-=(Duration duration) {
        offset -= duration;
        return *this;
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

}  // namespace chan

#endif
