#ifndef INCLUDED_CHAI_MUTEX
#define INCLUDED_CHAI_MUTEX

namespace chai {
class MutexImpl;

class Mutex {
    MutexImpl* d_impl_p;

  public:
    Mutex();
    ~Mutex();

    void lock();
    void unlock();
};

}  // namespace chai

#endif
