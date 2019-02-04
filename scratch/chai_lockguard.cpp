
#include <chai_lockguard.h>
#include <chai_mutex.h>

namespace chai {

LockGuard::LockGuard(Mutex& mutex) : d_mutex(mutex) { d_mutex.lock(); }

LockGuard::~LockGuard() { d_mutex.unlock(); }

}  // namespace chai