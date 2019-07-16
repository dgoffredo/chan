#include <chan/errors/error.h>
#include <chan/select/lasterror.h>

#include <cassert>

namespace chan {
namespace {

const Error fallbackError(
    "chan::select has not yet recorded an error on this thread.");

struct Storage {
    bool hasValue;
    union {
        char buffer[sizeof(Error)];
        // The following union members are included to ensure that `Storage`
        // has the maximum possible alignment, and thus an alignment compatible
        // with `Error`.  We can't have an actual `Error` member in the union,
        // because `Error` has a "non-trivial" constructor and C++98 forbids
        // that.  So, we use an adequately aligned buffer instead.
        // `long long` is omitted because it is not guaranteed to be available
        // in C++98.  `long double` will almost certainly have the largest
        // alignment (I can't think of a platform where it wouldn't).
        long double dummyLd;
        double      dummyD;
        void*       dummyP;
        long        dummyLi;
        int         dummyI;
    };
};

// In C++98, `thread_local` is not available.  As an alternative, `__thread` is
// a non-standard feature of many compilers.  I use it because:
// - The two most common non-Windows compilers, gcc and clang, support it.
// - Two "vendor compilers" I've worked with in the past, Solaris's CC and
//   AIX's xlc++, support it.
//
// I should also mention why I didn't use `pthread_setspecific` and
// `pthread_getspecific` along with a one-time `pthread_key_create`.  After
// all, that would be most portable.  The reason I don't use those functions
// is that both `pthread_key_create` and (notably) `pthread_setspecific` can
// fail due to lack of resources.  That would mean that `chan::setLastError`
// could fail, which would mean that users could not depend upon the value of
// `chan::lastError()`, which makes the whole feature useless.  By instead
// using a flavor of thread locality that the compiler is aware of, we
// eliminate all failure modes.
#if __cplusplus >= 201103
thread_local
#else
__thread
#endif
    Storage storage;

Error& value() {
    assert(storage.hasValue);
    return *reinterpret_cast<Error*>(&storage.buffer[0]);
}

}  // unnamed namespace

Error lastError() CHAN_NOEXCEPT {
    if (storage.hasValue) {
        return value();
    }
    else {
        return fallbackError;
    }
}

void setLastError(const Error& newError) CHAN_NOEXCEPT {
    if (storage.hasValue) {
        value() = newError;
    }
    else {
        // The destructor of this object will never get called, but that's ok,
        // because `chan::Error` contains only "plain old data" and has a
        // trivial destructor.  Its storage, on the other hand, will be taken
        // care of by the compiler's implementation of the thread-local storage
        // duration specifier.
        new (storage.buffer) Error(newError);
    }
}

}  // namespace chan
