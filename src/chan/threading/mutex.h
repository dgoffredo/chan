#ifndef INCLUDED_CHAN_THREADING_MUTEX
#define INCLUDED_CHAN_THREADING_MUTEX

namespace chan {
class MutexImpl;

class Mutex {
    MutexImpl* d_impl_p;

    Mutex(const Mutex&) /* = delete */;
    Mutex& operator=(const Mutex&) /* = delete */;

  public:
    Mutex();
    ~Mutex();

    void lock();
    void unlock();
};

}  // namespace chan

#endif
