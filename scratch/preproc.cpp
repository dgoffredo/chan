#include <chap_macros.h>

#define EVENT(N) CHAP_CAT(EVENT, N)
#define TYPENAME(N) typename EVENT(N)
#define DECLARE_SELECT(NUM_ARGS)                        \
    template <CHAP_MAP(TYPENAME, (CHAP_SEQ(NUM_ARGS)))> \
    int select(CHAP_MAP(EVENT, (CHAP_SEQ(NUM_ARGS))));

CHAP_MAPP(DECLARE_SELECT, (CHAP_SEQ(3)))

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
        return Select(events, end)();                                          \
    }

CHAP_MAPP(DEFINE_SELECT, (CHAP_SEQ(9)))

#undef DEFINE_SELECT
#undef REFIFY
#undef PARAM_DECL
#undef PARAM
#undef DECLARE_SELECT
#undef TYPENAME
#undef EVENT