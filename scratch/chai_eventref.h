#ifndef INCLUDED_CHAI_EVENTREF
#define INCLUDED_CHAI_EVENTREF

namespace chai {

struct EventRefVtable {
    IoEvent (*file)(void* instance);
    IoEvent (*fulfill)(void* instance, const IoEvent&);
    void (*cancel)(void* instance, const IoEvent&);
};

template <typename EVENT>
struct EventRefVtableImpl {
    static IoEvent file(void* instance) { return ref(instance).file(); }

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
    &EventRefVtableImpl<EVENT>::file, &EventRefVtableImpl<EVENT>::fulfill,
    &EventRefVtableImpl<EVENT>::cancel};

class EventRef {
    void*                 d_instance_p;
    const EventRefVtable* d_vtable_p;

  public:
    template <typename EVENT>
    EventRef(EVENT& event)
        : d_instance_p(&event),
          d_vtable_p(&EventRefVtableImpl<EVENT>::vtable) {}

    IoEvent file() { return d_vtable_p->file(d_instance_p); }

    IoEvent fulfill(const IoEvent& ioEvent) {
        return d_vtable_p->fulfill(d_instance_p, ioEvent);
    }

    void cancel(const IoEvent& ioEvent) {
        return d_vtable_p->cancel(d_instance_p, ioEvent);
    }
};

}  // namespace chai

#endif