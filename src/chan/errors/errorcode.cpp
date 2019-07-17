#include <chan/errors/errorcode.h>

namespace chan {
namespace {

const char* const messages[] = {
    // OTHER
    // `OTHER` is treated specially.  There's a constructor of `chan::Error`
    // that takes a diagnostic message.  In that case, the "code" is set to
    // `OTHER` and the "what" is set to the specified message.  However, if
    // `OTHER` is specified for the "code" without having specified a
    // "message," then the following default message is used instead.
    "An error occurred, but no diagnostic message was available.",

    // CREATE_PIPE
    "Unable to allocate a new pipe using POSIX pipe() in"
    " chan::PipePool::take().",

    // GET_PIPE_FLAGS
    "Unable to get pipe flags in drain() in chan::PipePool::giveBack.",

    // SET_PIPE_NONBLOCKING
    "Unable to set pipe to non-blocking in drain() in"
    " chan::PipePool::giveBack.",

    // DRAIN_PIPE
    "Unable to read remaining data (if any) from pipe buffer in"
    " chan::PipePool::giveBack.",

    // RESTORE_PIPE_FLAGS
    "Unable restore pipe's flags in drain() in chan::PipePool::giveBack.",

    // MUTEX_INIT
    "Unable to initialize mutex in chan::MutexImpl constructor.",

    // MUTEX_LOCK
    "Unable to lock mutex in chan::Mutex::lock().",

    // MUTEX_UNLOCK
    "Unable to unlock mutex in chan::Mutex::unlock().",

    // CURRENT_TIME
    "Unable to get the current time on the monotonic clock.",

    // POLL
    "The system IO multiplexing facility returned an error."
};

}  // unnamed namespace

const char* ErrorCode::message() const CHAN_NOEXCEPT {
    return messages[-int(value) - 1];
}

}  // namespace chan
