#ifndef INCLUDED_FULFILLMENTLOCKGUARD
#define INCLUDED_FULFILLMENTLOCKGUARD

#include <chan/threading/mutex.h>

#include <cassert>

namespace chan {

// When a visitor `ChanEvent` is interested in initiating a transfer with a
// sitter `ChanEvent`, it must first lock its own `SelectorFulfillment.mutex`
// and the sitter's.  In order to avoid a deadlock, the locks must be acquired
// in the same order.  This order is determined by the memory addresses of the
// mutexes.
// An event's `SelectorFulfillment.mutex` will already be locked when the event
// is called by `select`, and so the only situation in which the event needs to
// modify its own `SelectorFulfillment.mutex` is when the mutex is ordered
// _after_ the sitter's.  In that case, in order to acquire a lock on its mutex
// _after_ the sitter's, it must first unlock its mutex.  This is what avoids a
// deadlock.
class FulfillmentLockGuard {
    Mutex& mine;
    Mutex& theirs;
    bool   locked;

  public:
    FulfillmentLockGuard(Mutex& mine, Mutex& theirs)
    : mine(mine)
    , theirs(theirs)
    , locked(false) {
        // Locking order is detetermined by the mutexes' order in memory.
        if (&mine == &theirs) {
            // I assume `mine` is already locked, so there's nothing to do.
        }
        else if (&theirs < &mine) {
            // I assume `mine` is already locked, so in order to lock `mine`
            // _after_ locking `theirs`, I must first unlock `mine`.
            mine.unlock();

            theirs.lock();
            mine.lock();
        }
        else {
            assert(&mine < &theirs);

            // `mine` is already locked, so I can just lock `theirs`.
            theirs.lock();
        }

        locked = true;
    }

    ~FulfillmentLockGuard() {
        if (locked) {
            unlock();
        }
    }

    // Unlock this object.  The behavior is undefined unless this object is
    // currently locked.
    void unlock() {
        // `mine` will remain locked, so as long as `theirs != mine`, all I
        // need to do is unlock `theirs`.
        if (&mine != &theirs) {
            theirs.unlock();
        }

        locked = false;
    }
};

}  // namespace chan

#endif
