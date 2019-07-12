#ifndef INCLUDED_CHAN_THREADING_THREADLOCAL
#define INCLUDED_CHAN_THREADING_THREADLOCAL

// The class template `ThreadLocal` is meant to be used to define program-wide
// thread local variables by using the "curiously recurring template pattern."
//
// For example, suppose a library wishes to associate with each thread a
// `std::string` containing the most recent error message for that thread.
// The thread local variable could be defined in the following way:
//
//     class LastError : public ThreadLocal<std::string, LastError>
//     {};
//
// It could then be used in the following way:
//
//     void setError(const std::string& message) {
//         LastError::setValue(message);
//     }    
//
//     void clearError() {
//         if (!LastError::hasValue()) {
//             return;
//         }
//
//         // We could call `reset` each time, but to avoid having to allocate
//         // space for the `std::string` each time, we just clear its content
//         // instead (admittedly, it probably doesn't make any difference).
//         LastError::value().clear();
//     }
//
//     void reportError() {
//         if (!LastError::hasValue() || LastError::value().empty()) {
//             // Either no error has been set, or it has since been cleared.
//         }
// 
//         std::cerr << "Error occurred: " << LastError::value() << "\n";
//     }
//
// `ThreadLocal` allocates resources in the underlying system threading library
// during static initialization, and frees them during static destruction.

#include <cassert>

namespace chan {

// This opaque "key" type hides details of the underlying threading library.
struct ThreadLocalImplKey;

// This component-private class manages a type-agnostic thread local value.
class ThreadLocalImpl {
    void               (*deleter)(void*);
    ThreadLocalImplKey  *key;

  public:
    explicit ThreadLocalImpl(void (*deleter)(void*));

    ~ThreadLocalImpl();

    void *get();

    void set(void *newValue);

    void reset();

    template <typename VALUE>
    static void genericDeleter(void *object);
};

template <typename VALUE>
void ThreadLocalImpl::genericDeleter(void *object)
{
    assert(object);
    delete static_cast<VALUE*>(object);
}

// `class ThreadLocal` is the public part of this component.
template <typename VALUE, typename DERIVED>
class ThreadLocal {
    static ThreadLocalImpl impl;

    ThreadLocal() /* = delete */;

  public:
    // Return whether the calling thread currently has an object associated
    // with it.  If an error occurs, throw an exception.
    static bool hasValue();

    // Return a reference providing modifiable access to the object associated
    // with the calling thread.  The behavior is undefined unless the calling
    // thread has called `setValue` at least once since the last call to
    // `reset` or otherwise since program start.  If an error occurs, throw an
    // exception.
    static VALUE& value();

    // If `hasValue()` is false, then copy construct a thread local `VALUE`
    // using the specified `newValue`.  If `hasValue()` is true, then assign
    // to the current object the specified `newValue`.  `hasValue()` will
    // subsequently return `true`.  If an error occurs, throw an exception.
    static void setValue(const VALUE& newValue);

    // If `hasValue()` is true, destroy the object associated with the current
    // thread and free the storage it occupied.  `hasValue()` will subsequently
    // return `false`.  If an error occurs, throw an exception.
    static void reset();
};

template <typename VALUE, typename DERIVED>
ThreadLocalImpl
ThreadLocal<VALUE, DERIVED>::impl(&ThreadLocalImpl::genericDeleter<VALUE>);

template <typename VALUE, typename DERIVED>
bool ThreadLocal<VALUE, DERIVED>::hasValue()
{
    return impl.get();
}

template <typename VALUE, typename DERIVED>
VALUE& ThreadLocal<VALUE, DERIVED>::value()
{
    VALUE *ptr = static_cast<VALUE*>(impl.get());
    assert(ptr);
    return *ptr;
}

template <typename VALUE, typename DERIVED>
void ThreadLocal<VALUE, DERIVED>::setValue(const VALUE& newValue)
{
    if (VALUE *ptr = static_cast<VALUE*>(impl.get())) {
        *ptr = newValue;
    }
    else {
        impl.set(new VALUE(newValue));
    }
}

template <typename VALUE, typename DERIVED>
void ThreadLocal<VALUE, DERIVED>::reset()
{
    impl.reset();
}

}  // namespace chan

#endif
