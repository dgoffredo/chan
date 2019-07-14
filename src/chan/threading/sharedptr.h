#ifndef INCLUDED_CHAN_THREADING_SHAREDPTR
#define INCLUDED_CHAN_THREADING_SHAREDPTR

// If C++11's `std::shared_ptr` isn't available, we can take a deep breath and
// use a mutex.

#include <chan/threading/lockguard.h>
#include <chan/threading/mutex.h>

#include <algorithm>  // where std::swap is in C++98
#include <cassert>
#include <utility>  // where std::swap is in C++11

namespace chan {

struct SharedPtrControlBlock {
    Mutex mutex;
    int   referenceCount;

    SharedPtrControlBlock()
    : referenceCount(1) {
    }
};

template <typename OBJECT>
class SharedPtr {
    OBJECT*                object;
    SharedPtrControlBlock* controlBlock;

    void incrementRefCount() {
        if (controlBlock) {
            LockGuard lock(controlBlock->mutex);
            ++controlBlock->referenceCount;
        }
    }

    void decrementRefCount() {
        if (!controlBlock) {
            return;
        }

        int newRefCount;
        {
            LockGuard lock(controlBlock->mutex);
            newRefCount = --controlBlock->referenceCount;
        }

        if (newRefCount == 0) {
            delete object;
            delete controlBlock;
        }
    }

  public:
    SharedPtr()
    : object()
    , controlBlock() {
    }

    explicit SharedPtr(OBJECT* object)
    : object(object)
    , controlBlock(new SharedPtrControlBlock) {
    }

    SharedPtr(const SharedPtr& other)
    : object(other.object)
    , controlBlock(other.controlBlock) {
        incrementRefCount();
    }

    SharedPtr& operator=(const SharedPtr& other) {
        if (other.controlBlock == controlBlock) {
            return *this;
        }

        decrementRefCount();
        object       = other.object;
        controlBlock = other.controlBlock;
    }

    ~SharedPtr() {
        decrementRefCount();
    }

    friend void swap(SharedPtr& left, SharedPtr& right) {
        using std::swap;
        swap(left.object, right.object);
        swap(left.controlBlock, right.controlBlock);
    }

    OBJECT* get() const {
        return object;
    }

    OBJECT* operator->() const {
        return get();
    }

    OBJECT& operator*() const {
        OBJECT* const ptr = get();
        assert(ptr);
        return ptr;
    }

    operator void*() const {
        return get();
    }
};

}  // namespace chan

#endif
