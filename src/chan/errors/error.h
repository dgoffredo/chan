#ifndef INCLUDED_CHAN_ERRORS_ERROR
#define INCLUDED_CHAN_ERRORS_ERROR

#include <chan/errors/errorcode.h>
#include <chan/errors/noexcept.h>

#include <exception>

namespace chan {

class Error : public std::exception {
    ErrorCode rc;
    int       cerrno;
    char      msg[1024];

  public:
    explicit Error(const ErrorCode&);
    Error(const ErrorCode& code, int systemErrno);
    explicit Error(const char *message);

    ErrorCode code() const CHAN_NOEXCEPT;

    // The positive system error code (e.g. `errno`) associated with this
    // error, or `-1` if there is no associated system error code.
    int systemErrno() const CHAN_NOEXCEPT;

    const char *what() const CHAN_NOEXCEPT;
};

inline ErrorCode Error::code() const CHAN_NOEXCEPT {
    return rc;
}

inline int Error::systemErrno() const CHAN_NOEXCEPT {
    return cerrno;
}

inline const char *Error::what() const CHAN_NOEXCEPT {
    return msg;
}

}  // namespace chan

#endif
