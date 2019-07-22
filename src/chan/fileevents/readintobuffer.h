#ifndef INCLUDED_CHAN_FILEEVENTS_READINTOBUFFER
#define INCLUDED_CHAN_FILEEVENTS_READINTOBUFFER

#include <chan/errors/error.h>
#include <chan/fileevents/readevent.h>
#include <chan/select/lasterror.h>

#include <ostream>

namespace chan {

class ReadIntoBuffer {
    char* destination;
    int   numBytes;
    // `totalRead` is `mutable` for use in `ReadIntoBufferEvent::operator int`.
    mutable int* totalRead;

    friend class ReadIntoBufferEvent;

  public:
    ReadIntoBuffer(char* destination, int numBytes, int* totalRead)
    : destination(destination)
    , numBytes(numBytes)
    , totalRead(totalRead) {
    }

    template <typename READ_FUNC>
    ReadResult operator()(READ_FUNC doRead) {
        const int count = doRead(destination, numBytes);

        if (totalRead) {
            *totalRead = count;
        }

        return ReadResult::FULFILLED;
    }
};

class ReadIntoBufferEvent : public ReadEvent<ReadIntoBuffer> {
  public:
    ReadIntoBufferEvent(int   file,
                        char* destination,
                        int   numBytes,
                        int*  totalRead)
    : ReadEvent<ReadIntoBuffer>(
          file, ReadIntoBuffer(destination, numBytes, totalRead)) {
    }

    operator int() const {
        selectOnDestroy = false;

        int totalRead;
        handler.totalRead = &totalRead;

        if (select(*this)) {
            throw lastError();
        }

        return totalRead;
    }
};

inline std::ostream& operator<<(std::ostream&              stream,
                                const ReadIntoBufferEvent& event) {
    return stream << static_cast<int>(event);
}

inline ReadIntoBufferEvent read(int   file,
                                char* destination,
                                int   numBytes,
                                int*  totalRead = 0) {
    return ReadIntoBufferEvent(file, destination, numBytes, totalRead);
}

template <int N>
ReadIntoBufferEvent read(int file,
                         char (&destination)[N],
                         int* totalRead = 0) {
    return read(file, destination, N, totalRead);
}

}  // namespace chan

#endif
