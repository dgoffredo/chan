#ifndef INCLUDED_CHAN_FILES_PIPEPOOL
#define INCLUDED_CHAN_FILES_PIPEPOOL

#include <chan/files/pipe.h>
#include <chan/threading/mutex.h>

#include <vector>

namespace chan {

class PipePool {
    struct FreeListNode : public Pipe {
        FreeListNode* next;
    };

    Mutex         mutex;
    FreeListNode* freeList;

  public:
    PipePool();
    ~PipePool();

    // Return a pointer to a `Pipe` whose `referenceCount == 1`.  The `Pipe`
    // must be deallocated before this `PipePool` is destroyed.  A `Pipe` is
    // deallocated by passing it to `deallocate`.
    Pipe* allocate();

    // The behavior is undefined unless `pipe` was obtained from the result of
    // a previous call to `allocate` and whose `referenceCount` is zero.
    void deallocate(Pipe* pipe);
};

}  // namespace chan

#endif
