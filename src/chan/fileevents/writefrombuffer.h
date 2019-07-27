#ifndef INCLUDED_CHAN_FILEEVENTS_WRITEFROMBUFFER
#define INCLUDED_CHAN_FILEEVENTS_WRITEFROMBUFFER

#include <chan/errors/error.h>
#include <chan/fileevents/writeevent.h>
#include <chan/select/lasterror.h>

#include <cassert>
#include <ostream>
#include <string>
#if __cplusplus >= 201703
#include <string_view>
#endif

namespace chan {

class WriteFromBuffer {
    const char* source;
    const int   numBytes;
    int         numWritten;
    // `totalWritten` is `mutable` for use in
    // `WriteFromBufferEvent::operator int`.
    mutable int* totalWritten;

    friend class WriteFromBufferEvent;

  public:
    WriteFromBuffer(const char* source, int numBytes, int* totalWritten)
    : source(source)
    , numBytes(numBytes)
    , numWritten(0)
    , totalWritten(totalWritten) {
    }

    template <typename WRITE_FUNC>
    WriteResult operator()(WRITE_FUNC doWrite) {
        const int count = doWrite(source + numWritten, numBytes - numWritten);

        numWritten += count;
        assert(numWritten <= numBytes);

        if (count == 0) {
            return WriteResult::WAIT;
        }
        else {
            if (totalWritten) {
                *totalWritten = numWritten;
            }
            return WriteResult::FULFILLED;
        }
    }
};

class WriteFromBufferEvent : public WriteEvent<WriteFromBuffer> {
  public:
    WriteFromBufferEvent(int         file,
                         const char* source,
                         int         numBytes,
                         int*        totalWritten)
    : WriteEvent<WriteFromBuffer>(
          file, WriteFromBuffer(source, numBytes, totalWritten)) {
    }

    operator int() const {
        selectOnDestroy = false;

        int totalWritten;
        handler.totalWritten = &totalWritten;

        if (select(*this)) {
            throw lastError();
        }

        return totalWritten;
    }
};

inline std::ostream& operator<<(std::ostream&               stream,
                                const WriteFromBufferEvent& event) {
    return stream << static_cast<int>(event);
}

inline WriteFromBufferEvent write(int         file,
                                  const char* source,
                                  int         numBytes,
                                  int*        totalWritten = 0) {
    return WriteFromBufferEvent(file, source, numBytes, totalWritten);
}

inline WriteFromBufferEvent write(int                file,
                                  const std::string& source,
                                  int*               totalWritten = 0) {
    return WriteFromBufferEvent(
        file, source.c_str(), source.size(), totalWritten);
}

#if __cplusplus >= 201703
inline WriteFromBufferEvent write(int                     file,
                                  const std::string_view& source,
                                  int*                    totalWritten = 0) {
    return WriteFromBufferEvent(
        file, source.data(), source.size(), totalWritten);
}
#endif

}  // namespace chan

#endif
