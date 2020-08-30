#include <chan/errors/error.h>
#include <chan/errors/errorcode.h>
#include <chan/fileevents/ignoresigpipe.h>
#include <chan/fileevents/writeevent.h>

#include <errno.h>
#include <unistd.h>

namespace chan {

void* const forceLinkerToSeeIgnoreSigpipe = sigpipeIgnorer;

int WriteFunc::operator()(const char* source, int numBytes) const {
    if (numBytes == 0) {
        return numBytes;
    }

    assert(source);

    int numWritten = 0;
    while (numWritten < numBytes) {
        const ssize_t rc = ::write(fd, source + numWritten, numBytes - numWritten);
        if (rc != -1) {
            // successful write of `rc` bytes
            numWritten += rc;
            continue;
        }

        const int errorCode = errno;
        switch (errorCode) {
            case EAGAIN:
            case EPIPE:
                return numWritten;  // write would block, or no readers
            case EINTR:
                break;  // try again
            default:
                throw Error(ErrorCode::WRITE, errorCode);
        }
    }

    return numWritten;
}

}  // namespace chan
