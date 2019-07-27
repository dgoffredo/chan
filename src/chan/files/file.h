#ifndef INCLUDED_CHAN_FILES_FILE
#define INCLUDED_CHAN_FILES_FILE

#include <chan/fileevents/readintobuffer.h>
#include <chan/fileevents/readintostring.h>
#include <chan/fileevents/writefrombuffer.h>

#include <string>

namespace chan {

class File {
    int         fd;
    int         numBytesLastRead;
    int         numBytesLastWritten;

  public:
    // Create a `File` object that does not refer to any file.
    // `isOpen() == false`.
    File();

    // Create a `File` object that refers to a file using the specified
    // `fileDescriptor`.  `closeOnDestroy == false`.
    explicit File(int fileDescriptor);

    // The overload of `read` that do no specify a destination use this
    // `buffer` as its destination.
    std::string buffer;

    // If `closeOnDestroy == true`, then if `isOpen() == true` when this object
    // is destroyed, the file will be closed.
    bool closeOnDestroy;

    // Return whether this object refers to a file and has not been closed by
    // this object.
    bool isOpen() const;

    operator const void*() const {
        return isOpen() ? this : 0;
    }

    enum OpenMode {
        READ,
        WRITE,
        READ_WRITE
    };

    enum OpenResult {
        SUCCESS,
        BAD_PATH,
        NO_READERS,
        PERMISSION_DENIED,
        INSUFFICIENT_RESOURCES,
        OTHER
    };

    // Open the file at the specified `path` in the specified `mode`.  Return
    // `SUCCESS` if successful, after which `isOpen() == true` and
    // `closeOnDestroy == true`.  Return a non-`SUCCESS` `OpenResult` if an
    // error occurs.
    OpenResult open(const char*        path, OpenMode mode);
    OpenResult open(const std::string& path, OpenMode mode) {
        return open(path.c_str(), mode);
    }

    // Close the file.  The behavior is undefined unless `isOpen() == true`.
    // Subsequently, `closeOnDestroy == false`.
    void close();

    // Read up to the optionally specified `size` bytes into the optionally
    // specified `destination`. If `size` is not specified, then as many bytes
    // are read as possible without blocking.  If `destination` is not
    // specified, then bytes are read into storage managed by this object.  The
    // amount of bytes read is accessible by calling `gcount` after `read` but
    // before any subsequent call to `read`.  Throw an exception if an error
    // occurs.
    ReadIntoBufferEvent read(char* destination, int size);
    ReadIntoStringEvent read(std::string& destination);
    ReadIntoStringEvent read();

    // Write up to `size` of the bytes from the specified `source`.  The amount
    // of bytes written is accessible by calling `pcount` after `write` but
    // before any subsequent call to `write`.  Throw an exception if an error
    // occurs.
    WriteFromBufferEvent write(const char* source, int size);
    WriteFromBufferEvent write(const std::string& source);

    // Return the size, in bytes, of the most recent read.
    int gcount() {
        return numBytesLastRead;
    }

    // Return the size, in bytes, of the most recent write.
    int pcount() {
        return numBytesLastWritten;
    }
};

// Return a `File` object that reads from the standard input file.
File standardInput();

// Return a `File` object that writes to the standard output file.
File standardOutput();

// Return a `File` object that writes to the standard error file.
File standardError();

}  // namespace chan

#endif
