#include <chan/debug/trace.h>
#include <chan/errors/error.h>
#include <chan/files/filenonblockingguard.h>
#include <chan/files/pipepool.h>
#include <chan/threading/lockguard.h>

#include <cassert>
#include <cerrno>
#include <cstdlib>

#include <fcntl.h>      // fcntl()
#include <sys/types.h>  // suggested for use with fcntl()
#include <unistd.h>     // pipe()

namespace chan {
namespace {

void makePipe(int (&files)[2]) {
    if (::pipe(files)) {
        throw Error(ErrorCode::CREATE_PIPE, errno);
    }
}

void drain(int file) {
    // We first set the file to nonblocking, so that we can read any data that
    // remains without waiting for more writes.  We then read everything we
    // can, and finally restore the file's flags (make it blocking again).
    FileNonblockingGuard guard(file);

    // There will be at most a few bytes of data in there, so the buffer can be
    // small.
    char buffer[8];
    for (;;) {
        const int rcode = ::read(file, buffer, sizeof buffer);
        if (rcode == 0) {
            break;  // successfully read zero bytes, so it's now empty
        }
        else if (rcode == -1) {
            const int errorCode = errno;
            if (errorCode == EAGAIN) {
                break;  // no more data, so it's now empty
            }
            else if (errorCode != EINTR) {
                // `EINTR` would mean that `read` was interrupted by a signal,
                // which is fine, we just try again. If it was some other
                // error, though, then we have to fail.
                throw Error(ErrorCode::DRAIN_PIPE, errorCode);
            }
        }

        assert(rcode > 0);  // number of bytes read
    }
}

}  // namespace

PipePool::PipePool()
: freeList() {
}

PipePool::~PipePool() {
    // Close all of the files in the pool, and delete the free list.

    const FreeListNode* node = freeList;
    while (node) {
        const PipePair& pipePair = *node;

        ::close(pipePair.fromVisitor);
        ::close(pipePair.toSitter);
        ::close(pipePair.fromSitter);
        ::close(pipePair.toVisitor);

        const FreeListNode* next = node->next;
        delete node;
        node = next;
    }
}

PipePair* PipePool::allocate() {
    CHAN_WITH_LOCK(mutex) {
        if (freeList) {
            FreeListNode* result = freeList;
            freeList             = freeList->next;

            CHAN_TRACE("Allocating recycled pipes at ", result);

            ++result->referenceCount;
            assert(result->referenceCount == 1);
            return result;
        }
    }

    int towardsSitter[2];
    int towardsVisitor[2];
    makePipe(towardsSitter);
    makePipe(towardsVisitor);

    FreeListNode* node      = new FreeListNode;
    PipePair&     pipePair  = *node;
    pipePair.toSitter       = towardsSitter[1];
    pipePair.toVisitor      = towardsVisitor[1];
    pipePair.fromSitter     = towardsVisitor[0];
    pipePair.fromVisitor    = towardsSitter[0];
    pipePair.referenceCount = 1;

    CHAN_TRACE("Allocating new pipes at ", node);
    return node;
}

void PipePool::deallocate(PipePair* pipePair) {
    assert(pipePair);

    FreeListNode* node = static_cast<FreeListNode*>(pipePair);
    CHAN_TRACE("Deallocating pipes at ",
               node,
               ", whose reference count is ",
               pipePair->referenceCount);

    assert(pipePair->referenceCount == 0);

    // Clear any data left in the pipe buffers.
    drain(pipePair->fromVisitor);
    drain(pipePair->fromSitter);

    LockGuard lock(mutex);

    node->next = freeList;
    freeList   = node;
}

}  // namespace chan
