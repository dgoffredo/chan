#ifndef INCLUDED_CHAN_EVENT_EVENTREF
#define INCLUDED_CHAN_EVENT_EVENTREF

// `EventRef` is a type-erasing reference to any object that satisfies the
// _Event_ concept.  The _Event_ concept is a set of three member functions:
//
//     IoEvent file();
//
//     IoEvent fulfill(const IoEvent&);
//
//     void cancel(const IoEvent&);
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

namespace chan {

struct EventRefVtable {
    IoEvent (*file)(void* instance);
    IoEvent (*fulfill)(void* instance, const IoEvent&);
    void (*cancel)(void* instance, const IoEvent&);
};

template <typename EVENT>
struct EventRefVtableImpl {
    static IoEvent file(void* instance) {
        return ref(instance).file();
    }

    static IoEvent fulfill(void* instance, const IoEvent& ioEvent) {
        return ref(instance).fulfill(ioEvent);
    }

    static void cancel(void* instance, const IoEvent& ioEvent) {
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

  public:
    template <typename EVENT>
    EventRef(EVENT& event)
    : d_instance_p(&event)
    , d_vtable_p(&EventRefVtableImpl<EVENT>::vtable) {
    }

    IoEvent file() {
        return d_vtable_p->file(d_instance_p);
    }

    IoEvent fulfill(const IoEvent& ioEvent) {
        return d_vtable_p->fulfill(d_instance_p, ioEvent);
    }

    void cancel(const IoEvent& ioEvent) {
        return d_vtable_p->cancel(d_instance_p, ioEvent);
    }
};

}  // namespace chan

#endif
