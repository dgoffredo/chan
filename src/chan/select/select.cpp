#include <chan/errors/error.h>
#include <chan/errors/errorcode.h>
#include <chan/select/lasterror.h>
#include <chan/select/random.h>
#include <chan/select/select.h>
#include <chan/time/timepoint.h>

#include <errno.h>
#include <poll.h>

#include <algorithm>
#include <cassert>
#include <cstddef>  // std::size_t
#include <vector>

namespace chan {
namespace {

struct PollRecord {
    // pointer to the `struct` used to communicate with `::select`
    pollfd* pollFd;

    // the corresponding event
    EventRef event;

    // the most recent `IoEvent` returned by `event`
    IoEvent ioEvent;

    // This `state` is used for error handling.  We need to keep track of which
    // events need to be "cleaned up" when something else throws an exception.
    enum State { UNINITIALIZED, ACTIVE, DONE } state;

    explicit PollRecord(EventRef event, pollfd* pollFd)
    : pollFd(pollFd)
    , event(event)
    , ioEvent()
    , state(UNINITIALIZED) {
        assert(pollFd);
    }
};

// A `Selector` is the object that holds all of the state during a call to
// `chan::select`.
class Selector {
    // The elements of `pollFds` are aliased by the members of `records`.  Each
    // `PollRecord` contains a pointer to one of the `pollfd` in `pollFds`.
    // This way, `records` can be shuffled to enforce some kind of fairness.
    // The `Random15 generator` is used to do the shuffling.
    std::vector<pollfd>     pollFds;
    std::vector<PollRecord> records;
    Random15                generator;

    // The following functions return an iterator to the "winner" (event that
    // was fulfilled), or otherwise to `records.end()` if there was no winner.
    std::vector<PollRecord>::iterator doPoll();
    std::vector<PollRecord>::iterator handleTimeout();
    std::vector<PollRecord>::iterator handleFileEvent();

    Error handleError(const Error& caughtError);

  public:
    Selector(EventRef* events, const EventRef* end);

    // `operator()` does the selecting, returning the index of the first
    // fulfilled event, or otherwise a negative error code.
    int operator()();
};

Selector::Selector(EventRef* events, const EventRef* end)
: pollFds(end - events)
, records()
, generator(chan::systemRandom()) {
    records.reserve(pollFds.size());
    for (std::size_t i = 0; i < pollFds.size(); ++i) {
        const PollRecord record(events[i], &pollFds[i]);
        records.push_back(record);
    }
}

int Selector::operator()() try {
    // initial setup of `records`
    for (std::vector<PollRecord>::iterator it = records.begin();
         it != records.end();
         ++it) {
        PollRecord& record = *it;
        record.ioEvent     = record.event.file();
        record.state       = PollRecord::ACTIVE;
    }

    // Keep calling `::poll` until we either fulfill an event or throw an
    // exception.
    std::vector<PollRecord>::iterator winner;
    do {
        winner = doPoll();
    } while (winner == records.end());

    // Now that we have a winner, call `cancel` on all of the other events.
    for (std::vector<PollRecord>::iterator it = records.begin();
         it != records.end();
         ++it) {
        if (it != winner) {
            PollRecord& record = *it;
            record.event.cancel(record.ioEvent);
            record.state = PollRecord::DONE;
        }
    }

    assert(winner != records.end());
    assert(!pollFds.empty());
    assert(winner->pollFd >= &pollFds.front());
    assert(winner->pollFd <= &pollFds.back());

    // The index of the winner is determined by the position of the `pollfd`
    // in `pollFds`, because `records` will have been shuffled relative to that
    // initial order.
    const int winnerIndex = winner->pollFd - &pollFds.front();

    return winnerIndex;
}
catch (const Error& error) {
    const Error finalError = handleError(error);
    setLastError(finalError);
    return finalError.code();
}
catch (const std::exception& error) {
    const Error wrapper(error.what());
    const Error finalError = handleError(wrapper);
    setLastError(finalError);
    return finalError.code();
}
catch (...) {
    const Error error(ErrorCode::OTHER);
    const Error finalError = handleError(error);
    setLastError(finalError);
    return finalError.code();
}

void prepareRecord(PollRecord& record, const TimePoint*& deadline) {
    assert(record.pollFd);

    const IoEvent& io = record.ioEvent;
    if (io.timeout) {
        // It's a timeout event.
        record.pollFd->fd = -1;  // so `::poll` will ignore this entry

        if (!deadline || io.expiration < *deadline) {
            deadline = &io.expiration;
        }
    }
    else {
        // Otherwise, `io` is a read/write event on some file.
        record.pollFd->fd = io.file;
        if (io.read) {
            record.pollFd->events |=
                POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI;
        }
        if (io.write) {
            record.pollFd->events |= POLLOUT | POLLWRNORM | POLLWRBAND;
        }
    }
}

std::vector<PollRecord>::iterator Selector::doPoll() {
    // Set the fields of each 'pollfd' correctly based on each record's
    // `ioEvent`, and calculate the `deadline` (timeout), if any.
    const TimePoint* deadline = 0;  // null means "no deadline"
    for (std::vector<PollRecord>::iterator it = records.begin();
         it != records.end();
         ++it) {
        prepareRecord(*it, deadline);  // may modify both arguments
    }

    const int timeout =
        deadline ? std::max(0L, (*deadline - now()) / milliseconds(1)) : -1;

    assert(!pollFds.empty());

    switch (::poll(&pollFds.front(), pollFds.size(), timeout)) {
        case -1: {
            const int errorCode = errno;
            switch (errorCode) {
                case EINTR:
                    // signal was caught; fine, no winner yet
                    return records.end();
                default:
                    throw Error(ErrorCode::POLL, errorCode);
            }
        }
        case 0:
            return handleTimeout();
        default:
            return handleFileEvent();
    }
}

std::vector<PollRecord>::iterator Selector::handleTimeout() {
    chan::shuffle(records, generator);

    const TimePoint after = now();

    for (std::vector<PollRecord>::iterator it = records.begin();
         it != records.end();
         ++it) {
        PollRecord& record = *it;
        IoEvent&    io     = record.ioEvent;
        if (!io.timeout || io.expiration > after) {
            continue;  // not a timeout, or hasn't expired yet
        }

        // We found one of the events that expired.  Try to fulfill it.
        io = record.event.fulfill(io);
        if (io.fulfilled) {
            // we're done!
            record.state = PollRecord::DONE;
            return it;
        }
    }

    return records.end();  // no winner yet
}

std::vector<PollRecord>::iterator Selector::handleFileEvent() {
    chan::shuffle(records, generator);

    for (std::vector<PollRecord>::iterator it = records.begin();
         it != records.end();
         ++it) {
        PollRecord& record = *it;
        if (record.pollFd->revents == 0) {
            continue;  // this isn't one of the ready events
        }

        // Before calling `fulfill()` on the event, possibly set
        // response-only flags on the related `IoEvent`, so that
        // `fulfill()` has that information.
        IoEvent& io = record.ioEvent;

        if (record.pollFd->revents | POLLHUP) {
            io.hangup = true;
        }
        if (record.pollFd->revents | POLLERR) {
            io.error = true;
        }
        if (record.pollFd->revents | POLLNVAL) {
            io.invalid = true;
        }

        io = record.event.fulfill(record.ioEvent);
        if (io.fulfilled) {
            // we're done!
            record.state = PollRecord::DONE;
            return it;
        }
    }

    return records.end();  // no winner yet
}

Error Selector::handleError(const Error& originalError) {
    // To clean up, call `cancel` on any `PollRecord` current in the `ACTIVE`
    // state.  However, doing so could throw an exception, so possibly append
    // error messages as we go.
    bool caughtAnotherOne = false;

    Error combinedError(ErrorCode::SELECT_UNWINDING);
    combinedError.appendMessage(originalError.what());

    for (std::vector<PollRecord>::iterator it = records.begin();
         it != records.end();
         ++it) {
        PollRecord& record = *it;

        if (record.state != PollRecord::ACTIVE) {
            continue;
        }

        try {
            record.event.cancel(record.ioEvent);
        }
        catch (const std::exception& anotherError) {
            caughtAnotherOne = true;
            combinedError.appendMessage(anotherError.what());
        }
        catch (...) {
            caughtAnotherOne = true;
            combinedError.appendMessage(ErrorCode(ErrorCode::OTHER).message());
        }
    }

    if (caughtAnotherOne) {
        return combinedError;
    }
    else {
        return originalError;
    }
}

}  // namespace

int selectImpl(EventRef* eventsBegin, const EventRef* eventsEnd) {
    return Selector(eventsBegin, eventsEnd)();
}

}  // namespace chan
