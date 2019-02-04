
#include <chan/threading/threadlocal.h>
#include <chan/errors/error.h>
#include <chan/errors/errorcode.h>

#include <pthread.h>

#include <cassert>

namespace chan {

struct ThreadLocalImplKey {
    pthread_key_t key;
};

ThreadLocalImpl::ThreadLocalImpl(void (*deleteObject)(void*))
: deleter(deleteObject)
, key(new ThreadLocalImplKey)
{
    assert(key);

    if (const int rc = pthread_key_create(&key->key, deleter)) {
        throw Error(ErrorCode::CREATE_THREADLOCAL, rc);
    }
}

void *ThreadLocalImpl::get()
{
    assert(key);

    return pthread_getspecific(key->key);
}

void ThreadLocalImpl::set(void *newValue)
{
    assert(get() == 0);  // We don't want to leak a previously allocated value.
    assert(newValue);  // If you want to set to null, use `reset` instead.
    assert(key);

    if (const int rc = pthread_setspecific(key->key, newValue)) {
        throw Error(ErrorCode::SET_THREADLOCAL, rc);
    }
}

void ThreadLocalImpl::reset()
{
    void *const value = get();
    if (!value) {
        return;  // already nothing there
    }

    deleter(value);
    if (const int rc = pthread_setspecific(key->key, 0)) {
        throw Error(ErrorCode::CLEAR_THREADLOCAL, rc);
    }
}

}  // namespace chan
