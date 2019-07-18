#ifndef INCLUDED_CHAN_FILES_FILENONBLOCKINGGUARD
#define INCLUDED_CHAN_FILES_FILENONBLOCKINGGUARD

namespace chan {

class FileNonblockingGuard {
    int fd;
    int flags;

    static int  getFlags(int fd);
    static void restoreFlags(int fd, int flags);
    static void setNonblocking(int fd, int flags);

  public:
    explicit FileNonblockingGuard(int fd)
    : fd(fd)
    , flags(getFlags(fd)) {
        setNonblocking(fd, flags);
    }

    ~FileNonblockingGuard() {
        restoreFlags(fd, flags);
    }
};

}  // namespace chan

#endif
