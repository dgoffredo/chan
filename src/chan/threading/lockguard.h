#ifndef INCLUDED_CHAN_THREADING_LOCKGUARD
#define INCLUDED_CHAN_THREADING_LOCKGUARD

namespace chan {
class Mutex;

class LockGuard {
    Mutex& d_mutex;

  public:
    explicit LockGuard(Mutex& mutex);
    ~LockGuard();
};

}  // namespace chan

#endif
