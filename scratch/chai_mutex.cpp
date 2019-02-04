
#include <chai_error.h>
#include <chai_mutex.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>

#include <pthread.h>

namespace chai {

// ===============
// class MutexImpl
// ===============

class MutexImpl {
    pthread_mutex_t d_mutex;

  public:
    MutexImpl() {
        if (const int rcode = pthread_mutex_init(&d_mutex, 0)) {
            throw error(rcode,
                        "Unable to initialize mutex in chai::MutexImpl "
                        "constructor");
        }
    }

    int lock() { return pthread_mutex_lock(&d_mutex); }

    int unlock() { return pthread_mutex_unlock(&d_mutex); }
};

// -----------
// class Mutex
// -----------

Mutex::Mutex() : d_impl_p(new MutexImpl) {}

Mutex::~Mutex() { delete d_impl_p; }

void Mutex::lock() {
    if (const int rcode = d_impl_p->lock()) {
        throw error(rcode, "unable to lock mutex in chai::Mutex::lock()");
    }
}

void Mutex::unlock() {
    if (const int rcode = d_impl_p->unlock()) {
        throw error(rcode, "unable to unlock mutex in chai::Mutex::unlock()");
    }
}

}  // namespace chai
