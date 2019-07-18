#include <chan/errors/error.h>
#include <chan/errors/errorcode.h>
#include <chan/fileevents/readevent.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

namespace chan {

int ReadFunc::operator()(char* destination, int numBytes) const {
    int bytesWritten = 0;

    while (bytesWritten < numBytes) {
        const int rc = ::read(fd, destination, numBytes - bytesWritten);
        if (rc == -1) {
            switch (const int error = errno) {
                case EINTR:
                    continue;
#if EAGAIN != EWOULDBLOCK
                case EAGAIN:
#endif
                case EWOULDBLOCK:
                    return bytesWritten;
                default:
                    throw Error(ErrorCode::READ, error);
            }
        }
        else if (rc == 0) {
            // end of file
            return bytesWritten;
        }
        else {
            // `rc` is the (positive) number of bytes read.
            bytesWritten += rc;
            destination += rc;
        }
    }

    return bytesWritten;
}

}  // namespace chan
