#ifndef INCLUDED_CHAN_TIME
#define INCLUDED_CHAN_TIME

/// Intended Usage
//..
// const chan::ClockTime deadline = chan::now().addSeconds(0.01);
// something.waitUntil(deadline);
//..

#include <getRealTime.h>

namespace chan {

class ClockTime {
    // DATA
    double d_seconds;

    // FRIENDS
    friend ClockTime now();

    // PRIVATE CREATORS
    explicit ClockTime(double seconds) : d_seconds(seconds) {}

    // DELETED
    ClockTime();

  public:
    // MANIPULATORS
    ClockTime& addSeconds(double seconds) { d_seconds += seconds; }

    // ACCESSORS
    double seconds() const { return d_seconds; }
};

// FREE FUNCTIONS
ClockTime now();
// Return the current time as reported by a monotonic clock having an
// unspecified starting point.

}  // namespace chan

#endif