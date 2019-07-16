#include <chan/errors/error.h>
#include <chan/errors/errorcode.h>
#include <chan/time/timepoint.h>

#include <errno.h>
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

    TimePoint result;
    result.offset = seconds(spec.tv_sec) + nanoseconds(spec.tv_nsec);
    return result;
}

}  // namespace chan
