
#include <chan/errors/error.h>
#include <chan/threading/mutex.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>

#include <pthread.h>

namespace chan {

// `MutexImpl`, a component-private implementation class, is defined inline
// here.  Definitions of member functions of the public class, `Mutex`,
// follow.
class MutexImpl {
    pthread_mutex_t d_mutex;

  public:
    MutexImpl() {
        if (const int rcode = pthread_mutex_init(&d_mutex, 0)) {
            throw Error(ErrorCode::MUTEX_INIT, rcode);
        }
    }

    int lock() { return pthread_mutex_lock(&d_mutex); }

    int unlock() { return pthread_mutex_unlock(&d_mutex); }
};

Mutex::Mutex() : d_impl_p(new MutexImpl) {}

Mutex::~Mutex() { delete d_impl_p; }

void Mutex::lock() {
    if (const int rcode = d_impl_p->lock()) {
        throw Error(ErrorCode::MUTEX_LOCK, rcode);
    }
}

void Mutex::unlock() {
    if (const int rcode = d_impl_p->unlock()) {
        throw Error(ErrorCode::MUTEX_UNLOCK, rcode);
    }
}

}  // namespace chan
