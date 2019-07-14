#include <chan/errors/strerror.h>

#include <errno.h>
#include <string.h>

namespace chan {

int strError(int (*impl)(int, char*, size_t),
             int         error,
             char*       buffer,
             std::size_t bufferLength) {
    return impl(error, buffer, bufferLength);
}

int strError(char* (*impl)(int, char*, size_t),
             int         error,
             char*       buffer,
             std::size_t bufferLength) {
    errno = 0;  // `impl` reports errors by setting `errno`, so we must first
                // clear it in order to get a fair reading.
    const char* const msg = impl(error, buffer, bufferLength);
    const int         rc  = errno;

    if (msg != buffer) {
        strncpy(buffer, msg, bufferLength);
    }

    return rc;
}

}  // namespace chan
