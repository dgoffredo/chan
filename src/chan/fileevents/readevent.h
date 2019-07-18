#ifndef INCLUDED_CHAN_FILEEVENTS_READEVENT
#define INCLUDED_CHAN_FILEEVENTS_READEVENT

// This component provides a class template, `ReadEvent`, that satisfies the
// _Event_ concept.  `ReadEvent` represents the event of reading from a
// specified file descriptor.  The amount, if any, read from the file, to where
// the read data is written, and any side effects, are determined by a
// function-like object having the type that parameterizes `ReadEvent`.  Such
// an object is passed into the constructor of `ReadEvent`, and serves as its
// "handler."
//
// The handler must be invokable as if it were a function template having the
// following signature:
//
//     template <typename READ_FUNC>
//     chan::ReadResult ()(READ_FUNC doRead);
//
// The function returns `chan::ReadResult::FULFILLED` if read operations are
// complete, `chan::ReadResult::CONTINUE` if read operations are not yet
// complete, and throws an exception if an error occurs.
//
// The argument to the function is itself a function-like object invokable as
// if it were a function having the following signature:
//
//     int ()(char* destination, int numBytes);
//
// Invoking this function-like object reads up to `numBytes` from the file
// and copies them into storage starting at `destination`.  The function-like
// object then returns the non-negative number of bytes read, which could be
// zero.  If any error occurs, an exception will be thrown.

#include <chan/event/ioevent.h>
#include <chan/files/filenonblockingguard.h>
#include <chan/select/select.h>

#include <cassert>

namespace chan {

// This class is the return type of `ReadEvent` `HANDLERS`.  Possible values
// are:
// - `ReadResult::FULFILLED`, meaning that reading is done.
// - `ReadResult::CONTINUE`, meaning that we might want to read more later.
class ReadResult {
  public:
    enum Value { FULFILLED, CONTINUE };

  private:
    Value value;

  public:
    ReadResult(Value value)
    : value(value) {
    }

    operator Value() const {
        return value;
    }
};

class ReadFunc {
    int fd;

  public:
    explicit ReadFunc(int fd)
    : fd(fd) {
    }

    // Read at most `numBytes` from `fd` and copy them into `fd`.  Return the
    // number of bytes read.  Throw an exception if an error occurs.
    int operator()(char* destination, int numBytes) const;
};

template <typename HANDLER>
class ReadEvent {
    int          fd;
    HANDLER      handler;
    mutable bool selectOnDestroy;

  public:
    ReadEvent(int fd, HANDLER handler)
    : fd(fd)
    , handler(handler)
    , selectOnDestroy(true) {
    }

    ReadEvent(const ReadEvent& other)
    : fd(other.fd)
    , handler(other.handler)
    , selectOnDestroy(other.selectOnDestroy) {
        // If `other` thought that it was
        // responsible for calling `select` when
        // it's destroyed, it no longer is.
        other.selectOnDestroy = false;
    }

    ~ReadEvent() {
        if (selectOnDestroy) {
            chan::select(*this);
        }
    }

    IoEvent file() const {
        // We're participating with `select`, so there's no need to call
        // `select` when we're destroyed.
        selectOnDestroy = false;

        IoEvent event;
        event.read = true;
        event.file = fd;
        return event;
    }

    IoEvent fulfill(IoEvent event) {
        // Make sure that the file is in non-blocking mode, but restore
        // whatever flags it had previously once we're done with it.
        FileNonblockingGuard guard(fd);

        const ReadResult result = handler(ReadFunc(fd));
        if (result == ReadResult::FULFILLED) {
            event.fulfilled = true;
        }
        else {
            assert(result == ReadResult::CONTINUE);
        }

        return event;
    }

    void cancel(IoEvent) const {
    }
};  // namespace chan

}  // namespace chan

#endif
