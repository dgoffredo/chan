#ifndef INCLUDED_CHAN_THREADING_LOCKGUARD
#define INCLUDED_CHAN_THREADING_LOCKGUARD

#include <chan/threading/mutex.h>

#include <cassert>

namespace chan {

// `LockGuard`'s constructor locks a specified `Mutex`, and then `LockGuard`'s
// destructor unlocks the `Mutex`.
class LockGuard {
    Mutex& mutex;

  public:
    explicit LockGuard(Mutex& mutex)
    : mutex(mutex) {
        mutex.lock();
    }

    ~LockGuard() {
        mutex.unlock();
    }
};

// The `CHAN_WITH_LOCK` macro is syntactic sugar for scoping a block of code to
// hold a `LockGuard` on a specified mutex, e.g.
//
//     CHAN_WITH_LOCK(someMutex) {
//         // code...
//     }
//
// expands to the equivalent of:
//
//     {
//         LockGuard /*some name...*/(someMutex);
//         // code...
//     }

// `WithLockImpl` is an implementation detail of the `CHAN_WITH_LOCK` macro.
class WithLockImpl {
    Mutex&       mutex;
    mutable bool unlockOnDestroy;

  public:
    explicit WithLockImpl(Mutex& mutex)
    : mutex(mutex)
    , unlockOnDestroy(true) {
        mutex.lock();
    }

    ~WithLockImpl() {
        if (unlockOnDestroy) {
            mutex.unlock();
        }
    }

    WithLockImpl(const WithLockImpl& other)
    : mutex(other.mutex)
    , unlockOnDestroy(other.unlockOnDestroy) {
        assert(other.unlockOnDestroy);
        other.unlockOnDestroy = false;
    }

    operator bool() const {
        return true;
    }
};

#define CHAN_WITH_LOCK(MUTEX) \
    if (WithLockImpl cHaNwItHlOcKgUaRd = WithLockImpl(MUTEX))

}  // namespace chan

#endif
