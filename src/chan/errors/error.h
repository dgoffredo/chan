#ifndef INCLUDED_CHAN_ERRORS_ERROR
#define INCLUDED_CHAN_ERRORS_ERROR

#include <chan/errors/errorcode.h>
#include <chan/errors/noexcept.h>

#include <exception>
#include <iosfwd>

namespace chan {

class Error : public std::exception {
    ErrorCode rc;
    int       cerrno;
    char      msg[3 * 1024];

  public:
    // Populate `code()` and `what()` based on the specified `errorCode`, and
    // populate `systemErrno()` with -1, indicating N/A.
    explicit Error(const ErrorCode& errorCode) CHAN_NOEXCEPT;

    // Populate `code()` and `what()` based on the specified `errorCode`, and
    // populate `systemErrno()` with the specified `systemErrno`.
    Error(const ErrorCode& code, int systemErrno) CHAN_NOEXCEPT;

    // Populate `code()` with `ErrorCode::OTHER`, `what()` with a copy of the
    // specified null-terminated `message`, and populate `systemErrno()` with
    // -1, indicating N/A.
    explicit Error(const char* message) CHAN_NOEXCEPT;

    ErrorCode code() const CHAN_NOEXCEPT;

    // The positive system error code (e.g. `errno`) associated with this
    // error, or `-1` if there is no associated system error code.
    int systemErrno() const CHAN_NOEXCEPT;

    const char* what() const CHAN_NOEXCEPT;

    // Append to the message stored in this object two '\n' characters followed
    // by the specified null-terminated `extension`.  On success, return the
    // number of characters appended.  On failure, return a negative number.
    int appendMessage(const char* extension) CHAN_NOEXCEPT;
};

std::ostream& operator<<(std::ostream&, const Error&);

inline ErrorCode Error::code() const CHAN_NOEXCEPT {
    return rc;
}

inline int Error::systemErrno() const CHAN_NOEXCEPT {
    return cerrno;
}

inline const char* Error::what() const CHAN_NOEXCEPT {
    return msg;
}

}  // namespace chan

#endif
