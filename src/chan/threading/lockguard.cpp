
#include <chan/threading/lockguard.h>
#include <chan/threading/mutex.h>

namespace chan {

LockGuard::LockGuard(Mutex& mutex) : d_mutex(mutex) { d_mutex.lock(); }

LockGuard::~LockGuard() { d_mutex.unlock(); }

}  // namespace chan