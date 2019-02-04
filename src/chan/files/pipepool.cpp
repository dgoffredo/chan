#include <chan/errors/error.h>
#include <chan/threading/lockguard.h>
#include <chan/files/pipepool.h>

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstdlib>

#include <fcntl.h>      // fcntl()
#include <sys/types.h>  // suggested for use with fcntl()
#include <unistd.h>     // pipe(), sysconf()

namespace chan {
namespace {

const long k_PAGE_SIZE = sysconf(_SC_PAGE_SIZE);

bool available(const PipePair& pipes) {
    const bool result = pipes.toVisitor > 0 && pipes.fromSitter > 0;

    if (result) {
        // Since `PipePair` can be returned only from a visitor's perspective,
        // a sitter's perspective, or both, then at least the relevant pairs
        // will have the same sign, so we can get away with checking only one
        // from each pair.
        assert(pipes.fromVisitor > 0);
        assert(pipes.toSitter > 0);
    }

    return result;
}

void makePipe(int (&files)[2]) {
    if (pipe(files)) {
        throw Error(ErrorCode::CREATE_PIPE, errno);
    }
}

void markTaken(PipePair& pipes) {
    // The pipes are marked as being "taken" ("checked out") by negating them.
    // There are guaranteed to be enough negative values, and doing this
    // preserves what the positive value was.
    pipes.toSitter *= -1;
    pipes.toVisitor *= -1;
    pipes.fromSitter *= -1;
    pipes.fromVisitor *= -1;
}

class PipesMatch {
    const PipePair d_pipes;

  public:
    explicit PipesMatch(const PipePair& pipes) : d_pipes(pipes) {}

    bool operator()(const PipePair& pipes) const {
        using std::abs;
        // Absolute value of the bound `PipePair`, because some of the stored
        // descriptors might be negated if they're currently "checked out,"
        // but still we want to consider it the same `PipePair`.
        return d_pipes.toSitter == abs(pipes.toSitter) &&
               d_pipes.toVisitor == abs(pipes.toVisitor) &&
               d_pipes.fromSitter == abs(pipes.fromSitter) &&
               d_pipes.fromVisitor == abs(pipes.fromVisitor);
    }
};

PipesMatch matches(const PipePair& pipes) { return PipesMatch(pipes); }

void drain(int file) {
    // We first set the file to nonblocking, so that we can read any data that
    // remains without waiting for more writes.  We then read everything we
    // can, and finally restore the file's flags (make it blocking again).
    const int flags = fcntl(file, F_GETFL);

    if (flags == -1) {
        throw Error(ErrorCode::GET_PIPE_FLAGS, errno);
    }

    if (fcntl(file, F_SETFL, flags | O_NONBLOCK) == -1) {
        throw Error(ErrorCode::SET_PIPE_NONBLOCKING, errno);
    }

    // There will be at most a few bytes of data in there, so the buffer can be
    // small.
    char buffer[8];
    for (;;) {
        const int rcode = read(file, buffer, sizeof buffer);
        if (rcode == 0) {
            break;  // successfully read zero bytes, so it's now empty
        } else if (rcode == -1) {
            const int errorCode = errno;
            if (errorCode == EAGAIN) {
                break;  // no more data, so it's now empty
            } else if (errorCode != EINTR) {
                // `EINTR` would mean that `read` was interrupted by a signal,
                // which is fine, we just try again. If it was some other
                // error, though, then we have to fail.
                throw Error(ErrorCode::DRAIN_PIPE, errorCode);
            }
        }

        assert(rcode > 0);  // number of bytes read
    }

    // Restore the file's previous settings.
    if (fcntl(file, F_SETFL, flags) == -1) {
        throw Error(ErrorCode::RESTORE_PIPE_FLAGS, errno);
    }
}

}  // namespace

PipePool::~PipePool() {
    using std::abs;

    // Close all of the files in the pool.  `abs` is used so that it doesn't
    // matter whether the file is currently "checked out" (though you probably
    // have a bug if you're destroying the `PipePool` without having given back
    // all of its `PipePair`s).
    for (std::vector<PipePair>::const_iterator it = d_pipes.begin();
         it != d_pipes.end(); ++it) {
        const PipePair& pipes = *it;

        close(abs(pipes.fromVisitor));
        close(abs(pipes.toSitter));
        close(abs(pipes.fromSitter));
        close(abs(pipes.toVisitor));
    }
}

PipePair PipePool::take() {
    {
        LockGuard lock(d_mutex);

        const std::vector<PipePair>::iterator found =
            std::find_if(d_pipes.begin(), d_pipes.end(), available);

        if (found != d_pipes.end()) {
            PipePair result = *found;
            markTaken(*found);
            return result;  // RETURN
        }
    }

    int towardsSitter[2];
    int towardsVisitor[2];
    makePipe(towardsSitter);
    makePipe(towardsVisitor);

    PipePair pipes;
    pipes.toSitter    = towardsSitter[1];
    pipes.toVisitor   = towardsVisitor[1];
    pipes.fromSitter  = towardsVisitor[0];
    pipes.fromVisitor = towardsSitter[0];

    LockGuard lock(d_mutex);
    d_pipes.push_back(pipes);
    markTaken(d_pipes.back());

    return pipes;
}

void PipePool::giveBack(const PipePair& pipes, Whose whose) {
    LockGuard lock(d_mutex);

    const std::vector<PipePair>::iterator found =
        std::find_if(d_pipes.begin(), d_pipes.end(), matches(pipes));

    assert(found != d_pipes.end());
    PipePair& foundPipes = *found;

    using std::abs;
    // "Checked out" file descriptors will have been negated.  Make sure all
    // "given back" descriptors are positive.

    if (whose == SITTER || whose == BOTH) {
        foundPipes.fromVisitor = abs(foundPipes.fromVisitor);
        foundPipes.toVisitor   = abs(foundPipes.toVisitor);
    }

    if (whose == VISITOR || whose == BOTH) {
        foundPipes.fromSitter = abs(foundPipes.fromSitter);
        foundPipes.toSitter   = abs(foundPipes.toSitter);
    }

    // If all files in the `PipePair` have now been returned, it will be a
    // candidate for reuse.  Clear any data left in the pipe buffers.
    if (available(foundPipes)) {
        drain(foundPipes.fromVisitor);
        drain(foundPipes.fromSitter);
    }
}

}  // namespace chan
