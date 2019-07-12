#ifndef INCLUDED_CHAI_LOCKGUARD
#define INCLUDED_CHAI_LOCKGUARD

namespace chai {
class Mutex;

class LockGuard {
    // TODO

    // DATA
    Mutex& d_mutex;

  public:
    // CREATORS
    explicit LockGuard(Mutex& mutex);
    ~LockGuard();
};

}  // namespace chai

#endif
