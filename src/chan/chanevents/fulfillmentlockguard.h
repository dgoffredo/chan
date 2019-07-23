#ifndef INCLUDED_FULFILLMENTLOCKGUARD
#define INCLUDED_FULFILLMENTLOCKGUARD

#include <chan/threading/mutex.h>

#include <cassert>

namespace chan {

// TODO: explain
class FulfillmentLockGuard {
    Mutex& mine;
    Mutex& theirs;

  public:
    FulfillmentLockGuard(Mutex& mine, Mutex& theirs)
    : mine(mine)
    , theirs(theirs) {
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
    }

    ~FulfillmentLockGuard() {
        // `mine` will remain locked, so as long as `theirs != mine`, all I
        // need to do is unlock `theirs`.
        if (&mine != &theirs) {
            theirs.unlock();
        }
    }
};

}  // namespace chan

#endif
