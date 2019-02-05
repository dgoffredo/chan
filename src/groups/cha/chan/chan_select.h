#ifndef INCLUDED_CHAN_SELECT
#define INCLUDED_CHAN_SELECT

#include <chap_macros.h>

#include <bsl_cstdint.h>

namespace chan {

// The following variadic macro version of 'select' is commented out for C++03
// compatibility. However, it still serves as the contract for the overloads
// defined (in macros) below it.
/*

template <typename EVENT, typename... EVENTS>
bsl::size_t select(EVENT, EVENTS...);
    // TODO document

*/

#define EVENT(N) EVENT##N

#define TYPENAME(N) typename EVENT(N)

#define DECLARE_SELECT(NUM_ARGS)                             \
    template <CHAP_MAP(TYPENAME, CHAP_SEQ(NUM_ARGS))>        \
    bsl::size_t select(CHAP_MAP(EVENT, CHAP_SEQ(NUM_ARGS)));

CHAP_JOIN(;, CHAP_MAP(DECLARE_SELECT, CHAP_SEQ(9)));

// ============================================================================
//                         INLINE DEFINITIONS
// ============================================================================

namespace detail {

template <typename OBJECT>
T& deref(T *pointer)
{
    BSLS_ASSERT(pointer);
    return *pointer;
}

                            // ==============
                            // class Selector
                            // ==============

template <bsl::size_t numEvents>
class Selector {
    // TODO

    // TYPES
    struct Monitor {
        // TODO

        int                     d_file;
        btlso::EventType::Type  d_eventType;
        bsl::size_t             d_argumentIndex;
        Monitorface            *d_handler_p;
    };

    struct Deadline {
        // TODO

        bsls::TimeInterval  d_offsetFromEpoch;
        bsl::size_t         d_argumentIndex;
        Deadlineface       *d_handler_p;
    };

    // DATA
    Monitor                        d_monitors[numEvents];
        // 'd_monitors' contains at least enough storage for all of the file
        // monitors passed to 'select'.
    bsl::size_t                    d_numMonitors;
        // 'd_numMonitors' is the prefix of 'd_monitors' that is actually in
        // use. 'd_monitors' has space for 'numEvents' elements, but might not
        // use all of them, since some events are not file monitors (e.g.
        // timeouts).
    bdlb::NullableValue<Deadline>  d_deadline;
        // If a timeout or a deadline is among the arguments passed to
        // 'select', then the one that would time out soonest is stored in
        // 'd_deadline'. Otherwise, 'd_deadline' is null.
    Eventerface                   *d_currentSetupEvent_p;
        // The 'd_currentSetupEvent_p' argument is used only during
        // initialization. It points to the current 'Eventerface' instance as
        // we iterate through all of them in the constructor.
    bsl::size_t                    d_currentArgumentIndex;
        // 'd_currentArgumentIndex' is used during initialization as we iterate
        // through the input arguments to 'select'. It is also used to
        // communicate which non-timeout event, if any, was fulfilled.
    btlso::DefaultEventManager<>   d_eventManager;
        // 'd_eventManager' is a wrapper around some polling primitive on the
        // current platform. File events and optionally a timeout will be
        // registered with 'd_eventManager', and then this object's
        // 'operator()()' will block on 'd_eventManager.dispatch(...)'.

  public:
    // CREATORS
    explicit Selector(Eventerface (&events)[numEvents]);
        // TODO

    // MANIPULATORS
    void operator()(const bsl::pair<int, btlso::EventType::Type>& event);
        // TODO

    void operator()(const bsls::TimeInterval& epochalDeadline);
        // TODO

    void callback(Monitor *monitorPtr);
        // TODO

    bsl::size_t operator()();
        // TODO
};

                            // --------------
                            // class Selector
                            // --------------

template <bsl::size_t numEvents>
Selector<numEvents>::Selector(Eventerface (&events)[numEvents])
: d_numFiles()
, d_timeoutKind()
{
    for (bsl::size_t i = 0; i < numEvents; ++i)
    {
        Eventerface& current   = events[i];
        d_currentSetupEvent_p  = &current;
        d_currentArgumentIndex = i;
        current.setup().applyRaw(*this);
    }

    for (bsl::size_t i = 0; i < d_numMonitors; ++i)
    {
        Monitor& monitor = d_monitors[i];
        const int rc = d_eventManager.registerSocketEvent(
            monitor.d_file,
            monitor.d_eventType,
            bdlf::BindUtil::bind(&Selector::callback, this, &monitor));

        (void) rc;  // TODO: consider logging if registration fails
    }
}

template <bsl::size_t numEvents>
void Selector<numEvents>::callback(Monitor *monitorPtr)
{
    // TODO: refactor
    using bsl::make_pair;  // later

    Monitor&     monitor = deref(monitorPtr);
    Monitorface& handler = deref(monitor.d_handler_p);

    const bsl::pair<int, btlso::EventType::Type> result =
        handler.fulfill(monitor.d_file, monitor.d_eventType);

    if (result.first == -1)
    {
        // File descriptor '-1' is a special value meaning "fulfilled".
        // Cancel all monitored events, note the argument index of this
        // fulfilled monitor, and we're done.
        for (bsl::size_t i = 0; i < d_numMonitors; ++i) {
            Monitor& loser    = d_monitors[i];
            if (loser.d_handler_p == monitor.d_handler_p) {
                // Skip the one that just succeeded.
                continue;
            }
            Monitorface& slowPoke = deref(loser.d_handler_p);
            slowPoke.cancel(loser.d_file, loser.d_eventType);
        }

        d_eventManager.deregisterAll();

        d_currentArgumentIndex = monitor.d_argumentIndex;
    }
    else if (result !== make_pair(monitor.d_file, monitor.d_eventType))
    {
        // The (file, event) returned by 'fulfill' is not the same one as
        // was associated with this monitor before. Update the event
        // manager.
        d_eventManager.deregisterSocketEvent(monitor.d_file,
                                             monitor.d_eventType);

        const int rc = d_eventManager.registerSocketEvent(
            result.d_file,
            result.d_eventType,
            bdlf::BindUtil::bind(&Selector::callback, this, &monitor));

        (void) rc;  // TODO: consider logging if registration fails
    }
}

template <bsl::size_t numEvents>
bsl::size_t Selector<numEvents>::operator()()
{
    // TODO: refactor
    const int flags = 0;
    int       rc;

    if (!d_deadline.isNull()) {
        const bsls::TimeInterval& timeout =
            d_deadline.value().d_epochOffset;
        rc = d_eventManager.dispatch(timeout, flags);
    }
    else {
        rc = d_eventManager.dispatch(flags);
    }

    ///Possible Return Codes
    ///---------------------
    //: o '< -1' means an error occurred
    //: o '-1' would mean a signal interrupted the call if the appropriate
    //:   bit were set in 'flags', but it's not, so '-1' will not be
    //:   returned.
    //: o '0' means that a timeout occurred, and so only can happen if
    //    'd_deadline' is not null.
    //: o '> 0' is the number of events dispatched.

    if (rc < -1) {
        // TODO: Alternative possibilities: retry, throw?
        bsl::abort();                                                  // ABORT
    }
    else if (rc < 1) {
        BSLS_ASSERT(rc == 0 && !d_deadline.isNull());

        // Cancel monitored file events.
        for (bsl::size_t i = 0; i < d_numMonitors; ++i) {
            Monitor&     loser    = d_monitors[i];
            Monitorface& slowPoke = deref(loser.d_handler_p);
            slowPoke.cancel(loser.d_file, loser.d_eventType);
        }

        // The index in the argument list to 'select()' of this timeout
        // event.
        return d_deadline.value().d_argumentIndex;                    // RETURN
    }
    else {
        // The index in the argument list to 'select()' of the event that
        // was fulfilled.
        return d_argumentIndex;                                       // RETURN
    }
}

template <bsl::size_t numEvents>
void Selector<numEvents>::operator()(
    const bsl::pair<int, btlso::EventType::Type>& event)
{
    Monitor& monitor = d_monitors[d_numMonitors++];

    monitor.d_file          = event.first;
    monitor.d_eventType     = event.second;
    monitor.d_argumentIndex = d_currentArgumentIndex;
    monitor.d_handler_p     = static_cast<Monitorface*>(
                                                    d_currentSetupEvent_p);
}

template <bsl::size_t numEvents>
void Selector<numEvents>::operator()(const bsls::TimeInterval& epochalDeadline)
{
    if (d_deadline.isNull() ||
        d_deadline.value().d_offsetFromEpoch > epochalDeadline) 
    {
        Deadline& deadline = d_deadline.makeValue();

        deadline.d_offsetFromEpoch = epochalDeadline;
        deadline.d_argumentIndex   = d_currentArgumentIndex;
        deadline.d_handler_p       = 
            static_cast<Deadlineface*>(d_currentSetupEvent_p);
    }
}

}  // close detail namespace

#define EVENTERFACIFY(N) Eventerface(EVENT(N))

#define DEFINE_SELECT(NUM_ARGS)                                               \
    template <CHAP_MAP(TYPENAME, CHAP_SEQ(NUM_ARGS))>                         \
    bsl::size_t select(CHAP_MAP(EVENT, CHAP_SEQ(NUM_ARGS)))                   \
    {                                                                         \
        Eventerface events[] = {                                              \
            CHAP_MAP(EVENTERFACIFY, CHAP_SEQ(NUM_ARGS))                       \
        };                                                                    \
                                                                              \
        Selector<BDLB_ARRAYUTIL_LENGTH(events)> selector(events);             \
                                                                              \
        return selector();                                                    \
    }

#undef DEFINE_SELECT
#undef EVENTERFACIFY
#undef DECLARE_SELECT
#undef TYPENAME
#undef EVENT

}  // close package namespace

#endif
