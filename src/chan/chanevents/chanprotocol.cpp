#include <chan/chanevents/chanprotocol.h>
#include <chan/debug/trace.h>
#include <chan/errors/error.h>
#include <chan/errors/errorcode.h>

#include <errno.h>
#include <unistd.h>  // read, write

#include <cassert>

namespace chan {

namespace {

char toByte(ChanProtocolMessage message) {
    return message;
}

ChanProtocolMessage fromByte(char byte) {
    const ChanProtocolMessage result =
        static_cast<ChanProtocolMessage::Value>(byte);

    switch (result) {
        case ChanProtocolMessage::HI:
        case ChanProtocolMessage::READY:
        case ChanProtocolMessage::DONE:
        case ChanProtocolMessage::CANCEL:
        case ChanProtocolMessage::ERROR:
            break;
        default:
            assert(result == ChanProtocolMessage::POKE);
    }

    return result;
}

// The `toName` function is used only with `CHAN_TRACE`, so we conditionally
// define the function to appease `-W-unused-function`.
#ifdef CHAN_TRACE_ON
const char* toName(ChanProtocolMessage message) {
#define CASE(NAME)                  \
    case ChanProtocolMessage::NAME: \
        return #NAME;

    switch (message) {
        CASE(HI)
        CASE(READY)
        CASE(DONE)
        CASE(CANCEL)
        CASE(ERROR)
        default:
            assert(message == ChanProtocolMessage::POKE);
            return "POKE";
    }

#undef CASE
}
#endif  // #ifdef CHAN_TRACE_ON

}  // namespace

void writeMessage(int file, ChanProtocolMessage message) {
    const char payload  = toByte(message);
    const int  numBytes = sizeof(payload);

    CHAN_TRACE("- writing ", toName(message), " to fd ", file);

    for (;;) {
        const int rc = ::write(file, &payload, numBytes);
        if (rc == -1) {
            const int error = errno;
            if (error == EINTR) {
                continue;
            }

            throw Error(ErrorCode::PROTOCOL_WRITE, error);
        }

        // For an in-depth exploration of whether `write` can return zero when
        // a nonzero number of bytes was specified, see:
        // https://stackoverflow.com/questions/41904221/
        //          can-write2-return-0-bytes-written-and-what-to-do-if-it-does
        // I'm comfortable assuming that it won't.
        assert(rc == numBytes);
        return;
    }
}

ChanProtocolMessage readMessage(int file) {
    char      byte;
    const int numBytes = sizeof(byte);

    for (;;) {
        const int rc = ::read(file, &byte, numBytes);
        if (rc == -1) {
            const int error = errno;
            if (error == EINTR) {
                continue;
            }

            throw Error(ErrorCode::PROTOCOL_READ, error);
        }
        else if (rc == 0) {
            throw Error(ErrorCode::PROTOCOL_READ_EOF);
        }

        assert(rc == numBytes);

        CHAN_TRACE("- reading ", toName(fromByte(byte)), " from fd ", file);
        return fromByte(byte);
    }
}

}  // namespace chan
