#ifndef INCLUDED_CHAN_ERRORS_UNCAUGHTEXCEPTIONS
#define INCLUDED_CHAN_ERRORS_UNCAUGHTEXCEPTIONS

#include <chan/errors/noexcept.h>

namespace chan {

// Return the number of exceptions in the current thread that have been thrown
// or rethrown but have not yet entered their matching catch clauses.
int uncaughtExceptions() CHAN_NOEXCEPT;

}  // namespace chan

#endif
