#ifndef INCLUDED_CHAN_EVENT_EVENTCONTEXT
#define INCLUDED_CHAN_EVENT_EVENTCONTEXT

#include <chan/threading/mutex.h>
#include <chan/threading/sharedptr.h>

#include <cassert>
#include <ostream>

namespace chan {

// `EventKey` identifies an event within a `select` invocation.  It's meant to
// be opaque except for in the implementation of `select`.
typedef int EventKey;

// `SelectorFulfillment` is a means by which an event in one `select`
// invocation can check or set the fulfillment of an event in a different
// `select` invocation.
struct SelectorFulfillment {
    // Note that, by convention, `&mutex` (the address of the `mutex`) will be
    // used to determine the locking order among two or more
    // `SelectorFulfillment::mutex`.
    Mutex mutex;

    enum State {
        FULFILLABLE,   // not fulfilled, and fulfillment is allowed
        FULFILLED,     // has already been fulfilled
        UNFULFILLABLE  // not fulfilled, but fulfillment is not allowed
    };

    State state;

    // key of the fulfilled event; valid only if `state == FULFILLED`
    EventKey fulfilledEventKey;

    SelectorFulfillment()
    : mutex()
    , state(FULFILLABLE)
    , fulfilledEventKey(-1) {
    }
};

// An `EventContext` is given to each event passed to `select`.  Each event is
// passed an `EventContext` as an argument to the event's `file` method.  The
// `eventKey` within the `EventContext` refers to the event to whom's `file`
// method was passed the `EventContext` (i.e. "this is your key").  The
// `fulfillment` within the `EventContext` is a pointer to state common among
// all events within that invocation of `select`.  This way, `Chan`-related
// events in different `select` invocations can share their `EventContext` in
// order to allow an event in one `select` invocation to fulfill an event in a
// different `select` invocation.
struct EventContext {
    SharedPtr<SelectorFulfillment> fulfillment;
    // key of the event to which this `EventContext` was originally given
    EventKey eventKey;

    EventContext(const SharedPtr<SelectorFulfillment>& fulfillment,
                 EventKey                              eventKey)
    : fulfillment(fulfillment)
    , eventKey(eventKey) {
        assert(fulfillment);
    }

    EventContext()
    : fulfillment()
    , eventKey(-1) {
    }
};

inline std::ostream& operator<<(std::ostream&       stream,
                                const EventContext& context) {
    return stream << "[eventKey=" << context.eventKey
                  << " fulfillment=" << context.fulfillment.get() << "]";
}

}  // namespace chan

#endif
