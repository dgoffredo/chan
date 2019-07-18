#include <chan/select/random.h>

#if __cplusplus >= 201103

#include <random>

#else

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#endif

namespace chan {

#if __cplusplus >= 201103

int systemRandom() {
    return std::random_device()();
}

#else

int systemRandom() {
    int fd;
    do {
        fd = open("/dev/urandom", O_RDONLY);
    } while (fd == -1 && errno == EINTR);

    if (fd == -1) {
        return 0;
    }

    class FileGuard {
        int fd;

      public:
        explicit FileGuard(int fd)
        : fd(fd) {
        }

        ~FileGuard() {
            close(fd);
        }
    } guard(fd);

    // The choice of pointer types, below, is based on some version of GCC's
    // libstdc++, where the return value is referred to by a `void*` and then
    // incremented using a temporary `static_cast` to a `char*` (so that
    // pointer arithmetic can be used). Clang's libc++, on the other hand, just
    // uses a `reinterpret_cast` to `char*` to begin with.  I was at first
    // concerned about undefined behavior, but have since learned to stop
    // worrying and just do what the standard libraries do.
    int    result;
    void*  destination = &result;
    size_t bytesLeft   = sizeof(result);
    while (bytesLeft) {
        const int rc = read(fd, &result, bytesLeft);
        if (rc == -1) {
            if (errno == EINTR) {
                continue;
            }
            else {
                return 0;
            }
        }
        else if (rc == 0) {
            // It's not clear whether this is supposed to be possible with
            // "/dev/urandom", but both libc++ and libstdc++ consider it an
            // error, and so shall I.
            return 0;
        }
        else {
            // `rc` is the (positive) number of bytes read.
            bytesLeft -= rc;
            destination = static_cast<char*>(destination) + rc;
        }
    }

    return result;
}

#endif

}  // namespace chan
