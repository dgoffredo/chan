#ifndef INCLUDED_CHAN_SELECT_SELECT
#define INCLUDED_CHAN_SELECT_SELECT

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

#include <chan/event/eventref.h>
#include <chan/macros/macros.h>

namespace chan {

// The following C++11 function template prototype is commented out in order to
// support C++03.  However, the preprocessor macros below it declare equivalent
// overloads (up to a fixed maximum number of arguments).

/*
// Wait until any of the specified `events` is fulfilled and return the
// zero-based argument index of whichever is fulfilled first. If an error
// occurs, return a negative value indicating the kind of error (see
// `chan_errors.h`).
template <typename ...EVENTS>
int select(EVENTS ...events);
*/

#define EVENT(N)    CHAN_CAT(EVENT, N)
#define TYPENAME(N) typename EVENT(N)

#define DECLARE_SELECT(NUM_ARGS)                        \
    template <CHAN_MAP(TYPENAME, (CHAN_SEQ(NUM_ARGS)))> \
    int select(CHAN_MAP(EVENT, (CHAN_SEQ(NUM_ARGS))));

CHAN_MAPP(DECLARE_SELECT, (CHAN_SEQ(CHAN_MAX_ARITY)))

// This component-private function is an implementation detail of
// `chan::select`.
int selectImpl(EventRef* eventsBegin, const EventRef* eventsEnd);

#define PARAM(INDEX)      CHAN_CAT(event, INDEX)
#define PARAM_DECL(INDEX) EVENT(INDEX) PARAM(INDEX)
#define REFIFY(INDEX)     EventRef(PARAM(INDEX))

#define DEFINE_SELECT(NUM_ARGS)                                         \
    template <CHAN_MAP(TYPENAME, (CHAN_SEQ(NUM_ARGS)))>                 \
    int select(CHAN_MAP(PARAM_DECL, (CHAN_SEQ(NUM_ARGS)))) {            \
        EventRef events[] = { CHAN_MAP(REFIFY, (CHAN_SEQ(NUM_ARGS))) }; \
        const EventRef* const end =                                     \
            events + sizeof events / sizeof events[0];                  \
                                                                        \
        return selectImpl(events, end);                                 \
    }

CHAN_MAPP(DEFINE_SELECT, (CHAN_SEQ(CHAN_MAX_ARITY)))

#undef DEFINE_SELECT
#undef REFIFY
#undef PARAM_DECL
#undef PARAM
#undef DECLARE_SELECT
#undef TYPENAME
#undef EVENT

}  // namespace chan

#endif