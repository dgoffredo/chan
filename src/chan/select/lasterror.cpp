#include <chan/select/lasterror.h>
#include <chan/errors/error.h>
#include <chan/threading/threadlocal.h>

namespace chan {
namespace {

class LastError : public ThreadLocal<Error, LastError>
{};

const Error fallbackError("The most recent error local to this thread either "
                          "does not exist or could not be recorded.");

}  // unnamed namespace

Error lastError()
{
    if (LastError::hasValue()) {
        return LastError::value();
    }
    else {
        return fallbackError;
    }
}

void setLastError(const Error& newError)
{
    LastError::setValue(newError);
}

}  // namespace chan
