#include <chan/errors/error.h>
#include <chan/select/lasterror.h>
#include <chan/select/select.h>
#include <chan/time/duration.h>
#include <chan/time/timepoint.h>
#include <chan/timeevents/deadline.h>
#include <chan/timeevents/timeout.h>

#include <unistd.h>  // sleep

#include <iostream>

namespace {

void timePointAndDuration();
void timeoutAndDeadline();

}  // namespace

int main() {
    timeoutAndDeadline();
}

namespace {

void timeoutAndDeadline() {
    const chan::TimePoint when = chan::now() + chan::milliseconds(2);
    const int rc = chan::select(  chan::deadline(when)
                                , chan::deadline(when)
                                // , chan::timeout(chan::milliseconds(498))
                                // , chan::deadline(chan::now() + chan::nanoseconds(24))
                                // , chan::timeout(chan::milliseconds(499))
                                // , chan::timeout(chan::milliseconds(498))
                                );
    if (rc < 0) {
        std::cout << "An error occurred: " << chan::lastError() << "\n";
    }
    else {
        std::cout << "Event " << rc << " happened first.\n";
    }
}

void timePointAndDuration() {
    const chan::TimePoint before = chan::now();
    std::cout << "We begin " << (before - chan::TimePoint())
              << " since clock start.\n";
    sleep(3);
    std::cout << "We just slept for " << (chan::now() - before) << "\n";
}

}  // namespace