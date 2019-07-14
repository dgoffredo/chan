#include <chan/errors/errorcode.h>

namespace chan {
namespace {

const char *const messages[] = {
    // OTHER
    "",  // `OTHER` is treated specially

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

    // SET_THREADLOCAL
    "Unable to set a thread-local variable.",

    // CREATE_THREADLOCAL
    "Unable to create a key for a thread-local variable.",

    // CLEAR_THREADLOCAL
    "Unable to clear the value of a thread-local variable.",

    // CURRENT_TIME
    "Unable to get the current time on the monotonic clock."
};

}  // unnamed namespace

const char *ErrorCode::message() const CHAN_NOEXCEPT
{
    return messages[-int(value) - 1];
}

}  // namespace chan

