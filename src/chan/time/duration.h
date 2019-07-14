#ifndef INCLUDED_CHAN_TIME_DURATION
#define INCLUDED_CHAN_TIME_DURATION

#include <ostream>

namespace chan {

class Duration {
    long milliseconds;

    // This constructor is private.  To create `Duration` objects, use the
    // `seconds` and `milliseconds` functions.
    explicit Duration(long ms)
    : milliseconds(ms)
    {}

    friend Duration seconds(long quantity);
    friend Duration milliseconds(long quantity);

    friend std::ostream& operator<<(std::ostream& stream, Duration duration) {
        return stream << duration.milliseconds << "ms";
    }

    friend long operator/(Duration left, Duration right) {
        return left.milliseconds / right.milliseconds;
    }

    #define CHAN_DEFINE_COMPARISON(OP)                            \
        friend bool operator OP (Duration left, Duration right) { \
            return left.milliseconds OP right.milliseconds;       \
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
    : milliseconds()
    {}

    Duration& operator+=(Duration other) {
        milliseconds += other.milliseconds;
        return *this;
    }

    Duration& operator-=(Duration other) {
        milliseconds -= other.milliseconds;
        return *this;
    }

    Duration& operator*=(long factor) {
        milliseconds *= factor;
        return *this;
    }

    Duration& operator/=(long factor) {
        milliseconds /= factor;
        return *this;
    }
};

inline Duration seconds(long quantity) {
    return Duration(quantity * 1000);
}

inline Duration milliseconds(long quantity) {
    return Duration(quantity);
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

inline Duration operator-(Duration duration) {
    return Duration() - duration;
}

}  // namespace chan

#endif
