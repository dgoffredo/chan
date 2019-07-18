#include <chan/errors/error.h>
#include <chan/errors/errorcode.h>
#include <chan/files/filenonblockingguard.h>

#include <errno.h>
#include <fcntl.h>

namespace chan {

int FileNonblockingGuard::getFlags(int file) {
    const int flags = ::fcntl(file, F_GETFL);

    if (flags == -1) {
        throw Error(ErrorCode::GET_FILE_FLAGS, errno);
    }

    return flags;
}

void FileNonblockingGuard::restoreFlags(int file, int flags) {
    if (::fcntl(file, F_SETFL, flags) == -1) {
        throw Error(ErrorCode::RESTORE_FILE_FLAGS, errno);
    }
}

void FileNonblockingGuard::setNonblocking(int file, int flags) {
    if (::fcntl(file, F_SETFL, flags | O_NONBLOCK) == -1) {
        throw Error(ErrorCode::SET_FILE_NONBLOCKING, errno);
    }
}

}  // namespace chan