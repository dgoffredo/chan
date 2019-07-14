#ifndef INCLUDED_CHAN_ERRORS_STRERROR
#define INCLUDED_CHAN_ERRORS_STRERROR

// There are two flavors of `strerror_r` in the wild: the POSIX one returns an
// `int`, and the GNU one returns a `char*`.  The GNU one requires some extra
// handling, since the returned `char*` might refer to a buffer specified as
// an argument, or to some other string (this is fine if you just want to print
// it, but if you want to make sure it ends up in the buffer, you can't just
// copy, because overlapping arguments in `strncpy` is undefined).
//
// `strError`, below, takes as its first argument a pointer to the system's
// implementation of `strerror_r`.  There are then two overloads, one for each
// of the cases described above.  Only one will be used in a given build.  I
// like this better than conditinal compilation using `#ifdef`.  The functions
// are in their own component, instead of an implementation detail of another
// component, so that `-W-unused-function` does not complain.
//
// I should also mention why I didn't just use the portable `strerror_l`
// function instead.  The reason is that the prospect creating a locale and
// setting it up just right seemed trickier than overloading a function.

#include <cstddef>  // std::size_t

namespace chan {

int strError(int (*impl)(int, char*, size_t),
             int         error,
             char*       buffer,
             std::size_t bufferLength);

int strError(char* (*impl)(int, char*, size_t),
             int         error,
             char*       buffer,
             std::size_t bufferLength);

}  // namespace chan

#endif
