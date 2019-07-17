#include <chan/errors/error.h>
#include <chan/errors/strerror.h>

#include <stdio.h>
#include <string.h>

#include <cassert>
#include <ostream>

namespace chan {
namespace {

const char metaErrorMsg[] = "Unable to format error message.";

template <int n>
int copyMessage(char (&output)[n], const char* message) {
    const int result = snprintf(output, sizeof output, "%s", message);

    if (result < 0) {
        // If formatting the error message failed, then copy in a default
        // (unformatted) message.
        assert(sizeof(output) >= sizeof(metaErrorMsg));
        memcpy(output, metaErrorMsg, sizeof metaErrorMsg);
    }

    return result;
}

}  // unnamed namespace

Error::Error(const ErrorCode& code)
: rc(code)
, cerrno(-1) {
    copyMessage(msg, code.message());
}

Error::Error(const ErrorCode& code, int systemErrno)
: rc(code)
, cerrno(systemErrno) {
    const int offset1 = copyMessage(msg, code.message());
    if (offset1 < 0) {
        return;
    }

    // Keep track of where the trailing '\0' is and how many characters we have
    // left in the buffer.
    char* output = msg + offset1;
    int   size   = int(sizeof(msg)) - offset1;
    assert(size >= 0);

    const int offset2 = snprintf(output, size, " System error: ");
    if (offset2 < 0) {
        // We won't have the `strError` appended to this message, oh well.
        return;
    }

    output += offset2;
    size -= offset2;
    assert(size >= 0);

    strError(&strerror_r, systemErrno, output, size);
}

Error::Error(const char* message)
: rc(ErrorCode::OTHER)
, cerrno(-1) {
    assert(message);
    copyMessage(msg, message);
}

std::ostream& operator<<(std::ostream& stream, const Error& error) {
    return stream << error.what();
}

}  // namespace chan
