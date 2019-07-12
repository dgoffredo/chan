#ifndef INCLUDED_CHAN_SELECT
#define INCLUDED_CHAN_SELECT

// TODO
// For example, the following invocation:
//
//     chan::select(file.read(&buffer), done.recv(&inMsg), out.send(outMsg))
//
// would block until one of the three specified events is fulfilled. If the
// event `file.read(&buffer)` were fulfilled first, then the call to
// `chan::select` would return zero. If `done.recv(&inMsg)` were fulfilled
// first, then the number one would be returned. If `out.send(outMsg)` were
// fulfilled first, then the number two would be returned. If an error occurred
// before any event was fulfilled, then a negative number would be returned.

#include <chai_eventref.h>

#include <chap_macros.h>

namespace chan {

// FREE FUNCTIONS

// The following C++11 function template prototype is commented out in order to
// support C++03.  However, the preprocessor macros below it declare equivalent
// overloads (up to a fixed maximum number of arguments).

// Wait until any of the specified `events` is fulfilled and return the
// zero-based argument index of whichever is fulfilled first. If an error
// occurs, return a negative value indicating the kind of error (see
// `chan_errors.h`).
// template <typename ...EVENTS>
// int select(EVENTS ...events);

#define MAX_NUM_ARGS 9
#define EVENT(N) CHAP_CAT(EVENT, N)
#define TYPENAME(N) typename EVENT(N)
#define DECLARE_SELECT(NUM_ARGS)                        \
    template <CHAP_MAP(TYPENAME, (CHAP_SEQ(NUM_ARGS)))> \
    int select(CHAP_MAP(EVENT, (CHAP_SEQ(NUM_ARGS))));

CHAP_MAPP(DECLARE_SELECT, (CHAP_SEQ(MAX_NUM_ARGS)))

namespace detail {

// This component-private function is an implementation detail of
// `chan::select`.
int selectImpl(chai::EventRef* eventsBegin, const chai::EventRef* eventsEnd);

}  // namespace detail

// ============================================================================
//                             INLINE DEFINITIONS
// ============================================================================

#define PARAM(INDEX) CHAP_CAT(event, INDEX)
#define PARAM_DECL(INDEX) EVENT(INDEX) PARAM(INDEX)
#define REFIFY(INDEX) Ref(PARAM(INDEX))
#define DEFINE_SELECT(NUM_ARGS)                                                \
    template <CHAP_MAP(TYPENAME, (CHAP_SEQ(NUM_ARGS)))>                        \
    int select(CHAP_MAP(PARAM_DECL, (CHAP_SEQ(NUM_ARGS)))) {                   \
        typedef chai::EventRef Ref;                                            \
        Ref              events[] = {CHAP_MAP(REFIFY, (CHAP_SEQ(NUM_ARGS)))};  \
        const Ref* const end      = events + sizeof events / sizeof events[0]; \
                                                                               \
        return detail::selectImpl(events, end)();                              \
    }

CHAP_MAPP(DEFINE_SELECT, (CHAP_SEQ(MAX_NUM_ARGS)))

#undef DEFINE_SELECT
#undef REFIFY
#undef PARAM_DECL
#undef PARAM
#undef DECLARE_SELECT
#undef TYPENAME
#undef EVENT
#undef MAX_NUM_ARGS

}  // namespace chan

#endif