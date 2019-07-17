#ifndef INCLUDED_CHAN_TIME_TIMESPEC
#define INCLUDED_CHAN_TIME_TIMESPEC

// This component provides a plain old data class, `TimeSpec`, that represents
// a signed number of seconds together with a number of nanoseconds (naturally
// in the range `[0, 999 999 999]`).
// `TimeSpec` is used in the implementations of `Duration` and `TimePoint`, and
// is not intended for use by users of this library.  You're welcome to use it,
// though, if you like.

#include <chan/macros/macros.h>

#include <ctime>
#include <limits>
#include <ostream>

namespace chan {

struct TimeSpec {
    std::time_t seconds;
    long        nanoseconds;  // `long` is an unpopular type, but it's what
                              // POSIX uses, and so it's what we use here.
    TimeSpec()
    : seconds()
    , nanoseconds() {
    }

    TimeSpec(std::time_t seconds, long nanoseconds)
    : seconds(seconds)
    , nanoseconds(nanoseconds) {
        normalize();
    }

    TimeSpec& operator+=(TimeSpec other) {
        seconds += other.seconds;
        nanoseconds += other.nanoseconds;
        normalize();
        return *this;
    }

    TimeSpec& operator-=(TimeSpec other) {
        seconds -= other.seconds;
        nanoseconds -= other.nanoseconds;
        normalize();
        return *this;
    }

    TimeSpec& operator*=(long factor) {
        seconds *= factor;
        nanoseconds *= factor;
        normalize();
        return *this;
    }

    TimeSpec& operator/=(long factor) {
        seconds /= factor;
        nanoseconds /= factor;
        normalize();
        return *this;
    }

  private:
    enum {
        TIME_T__MUST__BE__SIGNED =
            1 / std::numeric_limits<std::time_t>::is_signed
    };

    void normalize() {
        const long oneBillion = CHAN_CAT(1, 000, 000, 000);

        if (nanoseconds < 0) {
            --seconds;
            nanoseconds += oneBillion;
        }
        else if (nanoseconds >= oneBillion) {
            ++seconds;
            nanoseconds -= oneBillion;
        }
    }
};

inline std::ostream& operator<<(std::ostream& stream, TimeSpec spec) {
    return stream << spec.seconds << "s " << spec.nanoseconds << "ns";
}

inline TimeSpec operator+(TimeSpec left, TimeSpec right) {
    return left += right;
}

inline TimeSpec operator-(TimeSpec left, TimeSpec right) {
    return left -= right;
}

inline TimeSpec operator*(TimeSpec spec, long factor) {
    return spec *= factor;
}

inline TimeSpec operator*(long factor, TimeSpec spec) {
    return factor * spec;  // calls the other `operator+`
}

inline TimeSpec operator/(TimeSpec spec, long factor) {
    return spec /= factor;
}

inline bool operator==(TimeSpec left, TimeSpec right) {
    return left.seconds == right.seconds &&
           left.nanoseconds == right.nanoseconds;
}

inline bool operator!=(TimeSpec left, TimeSpec right) {
    return left.seconds != right.seconds ||
           left.nanoseconds != right.nanoseconds;
}

inline bool operator<(TimeSpec left, TimeSpec right) {
    return left.seconds == right.seconds ? left.nanoseconds < right.nanoseconds
                                         : left.seconds < right.seconds;
}

inline bool operator<=(TimeSpec left, TimeSpec right) {
    return left.seconds == right.seconds
               ? left.nanoseconds <= right.nanoseconds
               : left.seconds < right.seconds;
}

inline bool operator>(TimeSpec left, TimeSpec right) {
    return left.seconds == right.seconds ? left.nanoseconds > right.nanoseconds
                                         : left.seconds > right.seconds;
}

inline bool operator>=(TimeSpec left, TimeSpec right) {
    return left.seconds == right.seconds
               ? left.nanoseconds >= right.nanoseconds
               : left.seconds > right.seconds;
}

}  // namespace chan

#endif
