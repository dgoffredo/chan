#ifndef INCLUDED_CHAN_FILES_PIPEPOOL
#define INCLUDED_CHAN_FILES_PIPEPOOL

#include <chan/files/pipepair.h>
#include <chan/threading/mutex.h>

#include <vector>

namespace chan {

class PipePool {
    struct FreeListNode : public PipePair {
        FreeListNode* next;
    };

    Mutex         mutex;
    FreeListNode* freeList;

  public:
    PipePool();
    ~PipePool();

    // Return a pointer to a `PipePair` whose `referenceCount == 1`.  The
    // `PipePair` must be deallocated before this `PipePool` is destroyed.  A
    // `PipePair` is deallocated by passing it to `deallocate`.
    PipePair* allocate();

    // The behavior is undefined unless `pipePair` was obtained from the result
    // of a previous call to `allocate` and whose `referenceCount` is zero.
    void deallocate(PipePair* pipePair);
};

}  // namespace chan

#endif
