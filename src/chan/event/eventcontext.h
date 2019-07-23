#ifndef INCLUDED_CHAN_EVENT_EVENTCONTEXT
#define INCLUDED_CHAN_EVENT_EVENTCONTEXT

#include <chan/threading/mutex.h>
#include <chan/threading/sharedptr.h>

namespace chan {

// `EventKey` identifies an event within a `select` invocation.
typedef int EventKey;

// Return the null value of `EventKey`.
inline EventKey nullEventKey() {
    return -1;
}

// `SelectorFulfillment` is a means by which an event in one `select`
// invocation can check or set the fulfillment of an event in a different
// `select` invocation.
struct SelectorFulfillment {
    // Note that, by convention, `&mutex` (the address of the `mutex`) will be
    // used to determine the locking order among two or more
    // `SelectorFulfillment::mutex`.
    Mutex mutex;
    // key of the fulfilled event, or null if no event is fulfilled (see
    // `isNull`, above).
    EventKey fulfilledEventKey;

    SelectorFulfillment()
    : mutex()
    , fulfilledEventKey(nullEventKey()) {
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
    }
};

}  // namespace chan

#endif
