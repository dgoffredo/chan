#include <chan/files/file.h>

#include <cassert>

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace chan {

File::File()
: fd(-1)
, numBytesLastRead(0)
, numBytesLastWritten(0)
, closeOnDestroy(false) {
}

File::File(int fileDescriptor)
: fd(fileDescriptor)
, numBytesLastRead(0)
, numBytesLastWritten(0)
, closeOnDestroy(false) {
}

bool File::isOpen() const {
    return fd != -1;
}

File::OpenResult File::open(const char* path, File::OpenMode mode) {
    int flags = 0;

    switch (mode) {
        case READ:
            flags = O_NONBLOCK | O_RDONLY;
            break;
        case WRITE:
            flags = O_NONBLOCK | O_WRONLY | O_CREAT;
            break;
        default:
            assert(mode == READ_WRITE);
            flags = O_NONBLOCK | O_RDWR | O_CREAT;
    }

    for (;;) {
        const int rc = ::open(path, flags);
        if (rc != -1) {
            // success
            fd             = rc;
            closeOnDestroy = true;
            return SUCCESS;
        }

        // An error occurred.  Figure out which one.
        switch (errno) {
            case EINTR:
                continue;  // retry
            case EACCES:
                return PERMISSION_DENIED;
            case ENXIO:
                return NO_READERS;
            case EMFILE:
            case ENFILE:
            case EOVERFLOW:
            case ENOMEM:
                return INSUFFICIENT_RESOURCES;
            case EISDIR:
            case ELOOP:
            case ENAMETOOLONG:
            case ENOENT:
            case ENOTDIR:
                return BAD_PATH;
            default:
                return OTHER;
        }
    }
}

void File::close() {
    assert(isOpen());

    int rc;
    do {
        rc = ::close(fd);
        // `::close` can fail for other reasons, but there's nothing we can do.
    } while (rc == -1 && errno == EINTR);

    fd             = -1;
    closeOnDestroy = false;
}

ReadIntoBufferEvent File::read(char* destination, int size) {
    return chan::read(fd, destination, size, &numBytesLastRead);
}

ReadIntoStringEvent File::read(std::string& destination) {
    return chan::read(fd, destination, &numBytesLastRead);
}

ReadIntoStringEvent File::read() {
    buffer.clear();
    return chan::read(fd, buffer, &numBytesLastRead);
}

WriteFromBufferEvent File::write(const char* source, int size) {
    return chan::write(fd, source, size, &numBytesLastWritten);
}

WriteFromBufferEvent File::write(const std::string& source) {
    return chan::write(fd, source, &numBytesLastWritten);
}

File standardInput() {
    return File(0);
}

File standardOutput() {
    return File(1);
}

File standardError() {
    return File(2);
}

}  // namespace chan
