#include <chan/errors/error.h>
#include <chan/errors/errorcode.h>
#include <chan/macros/macros.h>
#include <chan/time/timepoint.h>

#include <time.h>

#ifndef CLOCK_MONOTONIC
#error This library requires a system with a monotonic (and steady) clock
#endif

namespace chan {

TimePoint now() {
    timespec spec;
    if (clock_gettime(CLOCK_MONOTONIC, &spec)) {
        throw Error(ErrorCode::CURRENT_TIME, errno);
    }

    const time_t seconds      = spec.tv_sec;
    const long   milliseconds = spec.tv_nsec / CHAN_CAT(1, 000, 000);

    TimePoint result;
    result.seconds      = seconds;
    result.milliseconds = milliseconds;
    return result;
}

}  // namespace chan
