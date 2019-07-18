#ifndef INCLUDED_CHAN_FILEEVENTS_READINTOBUFFER
#define INCLUDED_CHAN_FILEEVENTS_READINTOBUFFER

#include <chan/fileevents/readevent.h>

namespace chan {

class ReadIntoBuffer {
    char* destination;
    int   numBytes;
    int*  totalRead;

  public:
    ReadIntoBuffer(char* destination, int numBytes, int* totalRead)
    : destination(destination)
    , numBytes(numBytes)
    , totalRead(totalRead) {
    }

    template <typename READ_FUNC>
    ReadResult operator()(READ_FUNC doRead) const {
        const int count = doRead(destination, numBytes);
        if (totalRead) {
            *totalRead = count;
        }

        return ReadResult::FULFILLED;
    }
};

inline ReadEvent<ReadIntoBuffer> read(int   file,
                                      char* destination,
                                      int   numBytes,
                                      int*  totalRead = 0) {
    return ReadEvent<ReadIntoBuffer>(
        file, ReadIntoBuffer(destination, numBytes, totalRead));
}

template <int N>
ReadEvent<ReadIntoBuffer> read(int file,
                               char (&destination)[N],
                               int* totalRead = 0) {
    return read(file, destination, N, totalRead);
}

}  // namespace chan

#endif
