#ifndef INCLUDED_CHAN_TIME_DURATION
#define INCLUDED_CHAN_TIME_DURATION

#include <chan/macros/macros.h>
#include <chan/time/timespec.h>

#include <cstdlib>
#include <ctime>
#include <limits>
#include <ostream>

namespace chan {

class Duration {
    TimeSpec spec;

    // This constructor is private.  To create `Duration` objects, use the
    // `seconds` and `milliseconds` functions.
    explicit Duration(TimeSpec spec)
    : spec(spec) {
    }

    friend Duration seconds(long quantity);
    friend Duration milliseconds(long quantity);
    friend Duration nanoseconds(long quantity);

    friend std::ostream& operator<<(std::ostream& stream, Duration duration) {
        return stream << duration.spec;
    }

    static long carefulDivide(Duration left, Duration right);
    friend long operator/(Duration left, Duration right);

#define CHAN_DEFINE_COMPARISON(OP)                           \
    friend bool operator OP(Duration left, Duration right) { \
        return left.spec OP right.spec;                      \
    }

    CHAN_DEFINE_COMPARISON(<)
    CHAN_DEFINE_COMPARISON(<=)
    CHAN_DEFINE_COMPARISON(>)
    CHAN_DEFINE_COMPARISON(>=)
    CHAN_DEFINE_COMPARISON(==)
    CHAN_DEFINE_COMPARISON(!=)

#undef CHAN_DEFINE_COMPARISON

  public:
    // A default constructed `Duration` represents a zero (empty) duration of
    // time.
    Duration()
    : spec() {
    }

    Duration& operator+=(Duration other) {
        spec += other.spec;
        return *this;
    }

    Duration& operator-=(Duration other) {
        spec -= other.spec;
        return *this;
    }

    Duration& operator*=(long factor) {
        spec *= factor;
        return *this;
    }

    Duration& operator/=(long factor) {
        spec /= factor;
        return *this;
    }
};

inline Duration seconds(long quantity) {
    return Duration(TimeSpec(quantity, 0));
}

inline Duration milliseconds(long quantity) {
    const std::ldiv_t seconds = std::ldiv(quantity, 1000);
    return Duration(
        TimeSpec(seconds.quot, seconds.rem * CHAN_CAT(1, 000, 000)));
}

inline Duration nanoseconds(long quantity) {
    return Duration(TimeSpec(0, quantity));
}

inline Duration operator+(Duration left, Duration right) {
    left += right;
    return left;
}

inline Duration operator-(Duration left, Duration right) {
    left -= right;
    return left;
}

inline Duration operator*(long factor, Duration duration) {
    duration *= factor;
    return duration;
}

inline Duration operator*(Duration duration, long factor) {
    return factor * duration;  // calls the other `operator*`
}

inline Duration operator/(Duration duration, long denominator) {
    duration /= denominator;
    return duration;
}

inline long operator/(Duration left, Duration right) {
    if (std::numeric_limits<long>::digits >= 63) {
        // This is quick and easy when `long` is 64-bits.
        const long billion = CHAN_CAT(1, 000, 000, 000);
        return (left.spec.seconds * billion + left.spec.nanoseconds) /
               (right.spec.seconds * billion + right.spec.nanoseconds);
    }
    else {
        // Spend a little more to avoid overflow for smaller `long` type.
        return Duration::carefulDivide(left, right);
    }
}

}  // namespace chan

#endif
