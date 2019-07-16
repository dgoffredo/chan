#ifndef INCLUDED_CHAN_SELECT_LASTERROR
#define INCLUDED_CHAN_SELECT_LASTERROR

// This component defines functions that provide access to a thread-local
// instance of `chan::Error`, used by `chan::select` to simplify error
// reporting.

#include <chan/errors/noexcept.h>

namespace chan {

class Error;

// Return a copy of the `Error` most recently recorded by `select` on the
// current thread. If there have not yet been any errors, then return an
// `Error` indicating so.
Error lastError() CHAN_NOEXCEPT;

// Copy the specified `newError` into thread-local storage if there is enough
// memory.  If there is not enough memory, do nothing.
void setLastError(const Error& newError) CHAN_NOEXCEPT;

}  // namespace chan

#endif
