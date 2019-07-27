#ifndef INCLUDED_CHAN_FILEEVENTS_WRITEEVENT
#define INCLUDED_CHAN_FILEEVENTS_WRITEEVENT

// This component provides a class template, `WriteEvent`, that satisfies the
// _Event_ concept.  `WriteEvent` represents the event of writing to a
// specified file descriptor.  It is the writing analog of `ReadEvent`.
//
// TODO

#include <chan/errors/uncaughtexceptions.h>
#include <chan/event/ioevent.h>
#include <chan/files/filenonblockingguard.h>
#include <chan/select/lasterror.h>
#include <chan/select/select.h>
#include <chan/time/duration.h>
#include <chan/time/timepoint.h>

#include <cassert>

namespace chan {

class EventContext;

// This class is the return type of `WriteEvent` `HANDLERS`.  Possible values
// are:
// - `WriteResult::FULFILLED`, meaning that writing is done.
// - `WriteResult::CONTINUE`, meaning that we might want to write more later.
// - `WriteResult::WAIT`, meaning that we're currently unable to write, but
//   want to try again later after some unspecified delay.
class WriteResult {
  public:
    enum Value { FULFILLED, CONTINUE, WAIT };

  private:
    Value value;

  public:
    WriteResult(Value value)
    : value(value) {
    }

    operator Value() const {
        return value;
    }
};

class WriteFunc {
    int fd;

  public:
    explicit WriteFunc(int fd)
    : fd(fd) {
    }

    // Write at most `numBytes` to `fd` from `source`.  Return the number of
    // bytes written. Throw an exception if an error occurs.
    int operator()(const char* source, int numBytes) const;
};

template <typename HANDLER>
class WriteEvent {
    int      fd;
    Duration brokenPipeTimeout;
    Duration handlerWaitTimeout;

  protected:
    HANDLER      handler;
    mutable bool selectOnDestroy;

  public:
    WriteEvent(int fd, HANDLER handler)
    : fd(fd)
    , handler(handler)
    , selectOnDestroy(true) {
        resetBrokenPipeTimeout();
        resetHandlerWaitTimeout();
    }

    WriteEvent(const WriteEvent& other)
    : fd(other.fd)
    , brokenPipeTimeout(other.brokenPipeTimeout)
    , handlerWaitTimeout(other.handlerWaitTimeout)
    , handler(other.handler)
    , selectOnDestroy(other.selectOnDestroy) {
        // If `other` thought that it was responsible for calling `select` when
        // it's destroyed, it no longer is.
        other.selectOnDestroy = false;
    }

    ~WriteEvent() CHAN_THROWS {
        if (selectOnDestroy && !uncaughtExceptions()) {
            if (select(*this)) {
                throw lastError();
            }
        }
    }

    void resetBrokenPipeTimeout() {
        brokenPipeTimeout = milliseconds(1);
    }

    void resetHandlerWaitTimeout() {
        handlerWaitTimeout = milliseconds(1);
    }

    void touch() CHAN_NOEXCEPT {
        // We're participating with `select`, so there's no need to call
        // `select` when we're destroyed.
        selectOnDestroy = false;
    }

    IoEvent file(const EventContext&) const {
        IoEvent event;
        event.write = true;
        event.file  = fd;
        return event;
    }

    IoEvent fulfill(IoEvent event) {
        // If `fd` is a pipe or FIFO (to which we wish to write), and if
        // there are no more readers, then `::poll` will set `POLLERR` and/or
        // `POLLHUP`.  In this case, I think it's reasonable to wait until
        // writing is possible, i.e. wait until another reader appears.  Since
        // we don't know when, if ever, this will happen, and we don't want to
        // "busy wait," instead we use a geometrically increasing series of
        // timeouts.  After each timeout, we wait for writability again, and if
        // it still fails, we increase the timeout up to some maximum amount.
        if (event.error || event.hangup) {
            IoEvent result;
            result.timeout    = true;
            result.expiration = now() + brokenPipeTimeout;

            // Increase timeout for next time, so long as we haven't maxed out.
            if (brokenPipeTimeout < seconds(1)) {
                brokenPipeTimeout *= 10;
            }

            return result;
        }

        // However, if the `fd` is writeable, then we can reset the timeout.
        resetBrokenPipeTimeout();

        // Make sure that the file is in non-blocking mode, but restore
        // whatever flags it had previously once we're done with it.
        FileNonblockingGuard guard(fd);

        switch (const WriteResult rc = handler(WriteFunc(fd))) {
            case WriteResult::FULFILLED:
                event.fulfilled = true;
                return event;
            case WriteResult::CONTINUE:
                // Since `handler` didn't return `WAIT`, we can reset that
                // timeout.
                resetHandlerWaitTimeout();
                return event;
            default: {
                assert(rc == WriteResult::WAIT);

                // This is analogous to the timeout situation described at the
                // beginning of this function, except that we use
                // `handlerWaitTimeout` instead of `brokenPipeTimeout`.
                IoEvent result;
                result.timeout    = true;
                result.expiration = now() + handlerWaitTimeout;

                // Increase timeout for next time, so long as we haven't maxed
                // out.
                if (handlerWaitTimeout < seconds(1)) {
                    handlerWaitTimeout *= 10;
                }

                return result;
            }
        }
    }

    void cancel(IoEvent) const {
    }
};
}  // namespace chan

#endif
