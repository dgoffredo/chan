#include <chan/errors/error.h>
#include <chan/select/lasterror.h>
#include <chan/threading/threadlocal.h>

namespace chan {
namespace {

class LastError : public ThreadLocal<Error, LastError> {};

const Error fallbackError("The most recent error local to this thread either "
                          "does not exist or could not be recorded.");

}  // unnamed namespace

Error lastError() CHAN_NOEXCEPT {
    if (LastError::hasValue()) {
        return LastError::value();
    }
    else {
        return fallbackError;
    }
}

void setLastError(const Error& newError) CHAN_NOEXCEPT try {
    LastError::setValue(newError);
}
catch (...) {
    // The only failure modes for `setValue` are `EINVAL`, for an invalid
    // `pthread_key_t`, and `ENOMEM`, if we run out of memory.  Either way,
    // there's nothing we can do, so ignore all errors.
}

}  // namespace chan
