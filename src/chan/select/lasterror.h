#ifndef INCLUDED_CHAN_SELECT_LASTERROR
#define INCLUDED_CHAN_SELECT_LASTERROR

// This component defines functions that provide access to a thread-local
// instance of `chan::Error`, used by `chan::select` to simplify error
// reporting.

namespace chan {

class Error;

// Return a copy of the `Error` most recently recorded by this library on the
// current thread. If there have not yet been any errors, then return an
// `Error` indicating so.
Error lastError();

// Copy the specified `newError` into thread-local storage.  If an error
// occurs, throw an exception.
void setLastError(const Error& newError);

}  // namespace chan

#endif
