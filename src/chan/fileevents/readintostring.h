#ifndef INCLUDED_CHAN_FILEEVENTS_READINTOSTRING
#define ICNLUDED_CHAN_FILEEVENTS_READINTOSTRING

#include <chan/errors/error.h>
#include <chan/fileevents/readevent.h>
#include <chan/select/lasterror.h>
#include <chan/select/select.h>

#include <ostream>
#include <string>

namespace chan {

class ReadIntoString {
  public:
    std::string& destination;

  private:
    int* totalRead;

  public:
    ReadIntoString(std::string& destination, int* totalRead)
    : destination(destination)
    , totalRead(totalRead) {
    }

    template <typename READ_FUNC>
    ReadResult operator()(READ_FUNC doRead) const {
        destination.clear();

        char buffer[4096];  // typical pipe buffer size on Linux
        int  totalCount = 0;
        for (;;) {
            const int count = doRead(buffer, sizeof(buffer));
            if (count == 0) {
                break;  // no more to read
            }

            destination.append(buffer, count);
            totalCount += count;
        }

        if (totalRead) {
            *totalRead = totalCount;
        }

        return ReadResult::FULFILLED;
    }
};

class ReadIntoStringEvent : public ReadEvent<ReadIntoString> {
  public:
    ReadIntoStringEvent(int file, std::string& destination, int* totalRead)
    : ReadEvent<ReadIntoString>(file, ReadIntoString(destination, totalRead)) {
    }

    operator std::string&() const {
        selectOnDestroy = false;

        if (chan::select(*this)) {
            throw chan::lastError();
        }

        return handler.destination;
    }
};

inline std::ostream& operator<<(std::ostream&              stream,
                                const ReadIntoStringEvent& event) {
    return stream << static_cast<std::string&>(event);
}

inline ReadIntoStringEvent read(int          file,
                                std::string& destination,
                                int*         totalRead = 0) {
    return ReadIntoStringEvent(file, destination, totalRead);
}

}  // namespace chan

#endif
