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
    int  totalRead = 0;  // for debugging
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
        totalRead += rcode;
    }

    CHAN_TRACE("PipePool::drain read a total of ",
               totalRead,
               " bytes from file descriptor ",
               file);
}

}  // namespace

PipePool::PipePool()
: freeList() {
}

PipePool::~PipePool() {
    // Close all of the files in the pool, and delete the free list.

    const FreeListNode* node = freeList;
    while (node) {
        const Pipe& pipe = *node;

        ::close(pipe.fromVisitor);
        ::close(pipe.toSitter);

        const FreeListNode* next = node->next;
        delete node;
        node = next;
    }
}

Pipe* PipePool::allocate() {
    CHAN_WITH_LOCK(mutex) {
        if (freeList) {
            FreeListNode* result = freeList;
            freeList             = freeList->next;

            CHAN_TRACE("Allocating recycled pipe at ",
                       result,
                       " (",
                       result->toSitter,
                       " -> ",
                       result->fromVisitor,
                       ")");

            ++result->referenceCount;
            assert(result->referenceCount == 1);
            return result;
        }
    }

    int towardsSitter[2];
    makePipe(towardsSitter);

    FreeListNode* node  = new FreeListNode;
    Pipe&         pipe  = *node;
    pipe.fromVisitor    = towardsSitter[0];
    pipe.toSitter       = towardsSitter[1];
    pipe.referenceCount = 1;

    CHAN_TRACE("Allocating new pipe at ",
               node,
               " (",
               pipe.toSitter,
               " -> ",
               pipe.fromVisitor,
               ")");
    return node;
}

void PipePool::deallocate(Pipe* pipe) {
    assert(pipe);

    // The only way that the following `static_cast` could be valid is if
    // `pipe` really does refer to a `FreeListNode`.  Together with the fact
    // that the definition of `FreeListNode` is private, that is why the
    // contract of this function says: "The behavior is undefined unless `pipe`
    // was obtained from the result of a previous call to `allocate` [...]".
    FreeListNode* node = static_cast<FreeListNode*>(pipe);

    CHAN_TRACE("Deallocating pipe at ",
               node,
               ", whose reference count is ",
               pipe->referenceCount,
               " (",
               pipe->toSitter,
               " -> ",
               pipe->fromVisitor,
               ")");

    assert(pipe->referenceCount == 0);

    // Clear any data left in the pipe buffers.
    drain(pipe->fromVisitor);

    LockGuard lock(mutex);

    node->next = freeList;
    freeList   = node;
}

}  // namespace chan
