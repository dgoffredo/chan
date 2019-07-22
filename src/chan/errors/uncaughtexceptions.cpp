#include <chan/errors/uncaughtexceptions.h>

#include <exception>

// The implementation will depend on the version of C++.  For reference, see:
// https://en.cppreference.com/w/cpp/error/uncaught_exception

namespace chan {

int uncaughtExceptions() CHAN_NOEXCEPT {
#if __cplusplus >= 201703
    return std::uncaught_exceptions();
#else
    return std::uncaught_exception();
#endif
}

}  // namespace chan
