#ifndef INCLUDED_CHAN_EVENT_EVENTREF
#define INCLUDED_CHAN_EVENT_EVENTREF

// `EventRef` is a type-erasing reference to any object that satisfies the
// _Event_ concept.  The _Event_ concept is a set of three member functions:
//
//     IoEvent file();
//
//     IoEvent fulfill(IoEvent);
//
//     void cancel(IoEvent);
//
// See this package's `README.md` file for an explanation of each of these
// member functions.
//
// `EventRef` satisfies the _Event_ concept.  Its member functions are
// implemented by calling the corresponding member functions on the referred-to
// object through a pointer to that object.
//
// For example,
//
//     SomeEventType event;
//     EventRef      ref(event);
//     IoEvent       io = ref.file();
//
// Note that since `EventRef` has reference semantics, the lifetime of the
// object to which it refers must exceed the `EventRef` lifetime.

#include <chan/event/ioevent.h>

#include <cassert>
#include <ostream>

namespace chan {

struct EventRefVtable {
    IoEvent (*file)(void* instance);
    IoEvent (*fulfill)(void* instance, IoEvent);
    void (*cancel)(void* instance, IoEvent);
};

template <typename EVENT>
struct EventRefVtableImpl {
    static IoEvent file(void* instance) {
        return ref(instance).file();
    }

    static IoEvent fulfill(void* instance, IoEvent ioEvent) {
        return ref(instance).fulfill(ioEvent);
    }

    static void cancel(void* instance, IoEvent ioEvent) {
        ref(instance).cancel(ioEvent);
    }

    static const EventRefVtable vtable;

  private:
    static EVENT& ref(void* instance) {
        assert(instance);
        return *static_cast<EVENT*>(instance);
    }
};

template <typename EVENT>
const EventRefVtable EventRefVtableImpl<EVENT>::vtable = {
    &EventRefVtableImpl<EVENT>::file,
    &EventRefVtableImpl<EVENT>::fulfill,
    &EventRefVtableImpl<EVENT>::cancel
};

class EventRef {
    void*                 d_instance_p;
    const EventRefVtable* d_vtable_p;

    friend std::ostream& operator<<(std::ostream&   stream,
                                    const EventRef& event) {
        return stream << "[instance=" << event.d_instance_p
                      << " vtable=" << event.d_vtable_p << "]";
    }

  public:
    // Mustn't forget to specifically define copy-from-const and
    // copy-from-non-const constructors, or else the constructor template will
    // be selected instead.
    EventRef(const EventRef& other) /* = default */
    : d_instance_p(other.d_instance_p)
    , d_vtable_p(other.d_vtable_p) {
    }

    EventRef(EventRef& other)
    : d_instance_p(other.d_instance_p)
    , d_vtable_p(other.d_vtable_p) {
    }

    template <typename EVENT>
    explicit EventRef(EVENT& event)
    : d_instance_p(&event)
    , d_vtable_p(&EventRefVtableImpl<EVENT>::vtable) {
    }

    IoEvent file() {
        return d_vtable_p->file(d_instance_p);
    }

    IoEvent fulfill(IoEvent ioEvent) {
        return d_vtable_p->fulfill(d_instance_p, ioEvent);
    }

    void cancel(IoEvent ioEvent) {
        return d_vtable_p->cancel(d_instance_p, ioEvent);
    }
};

}  // namespace chan

#endif
