#ifndef INCLUDED_CHAN_MACROS_MACROS
#define INCLUDED_CHAN_MACROS_MACROS

// This header contains preprocessor macros used to generate code elsewhere in
// this library. The maximum argument list length supported by these macros is
// currently nine.

//     CHAN_CAT(LEFT, RIGHT)
//
// expands to
//
//    LEFTRIGHT
//
// However, if either of LEFT or RIGHT is a macro invocation, then it will be
// expanded before pasting.
#define CHAN_CAT_(LEFT, RIGHT) LEFT##RIGHT
#define CHAN_CAT(LEFT, RIGHT) CHAN_CAT_(LEFT, RIGHT)

//     CHAN_SEQ(N)
//
// expands to
//
//     1, 2, 3, ..., N
#define CHAN_SEQ1 1
#define CHAN_SEQ2 CHAN_SEQ1, 2
#define CHAN_SEQ3 CHAN_SEQ2, 3
#define CHAN_SEQ4 CHAN_SEQ3, 4
#define CHAN_SEQ5 CHAN_SEQ4, 5
#define CHAN_SEQ6 CHAN_SEQ5, 6
#define CHAN_SEQ7 CHAN_SEQ6, 7
#define CHAN_SEQ8 CHAN_SEQ7, 8
#define CHAN_SEQ9 CHAN_SEQ8, 9

#define CHAN_SEQ(N) CHAN_CAT(CHAN_SEQ, N)

//     CHAN_LENGTH(1, 2, 3, ..., N)
//
// expands to
//
//     N
#define CHAN_LENGTH_(A9, A8, A7, A6, A5, A4, A3, A2, A1, LENGTH, ...) LENGTH
#define CHAN_LENGTH(...) CHAN_LENGTH_(__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1)

#define CHAN_FIRST(FIRST, ...) FIRST
#define CHAN_REST(FIRST, ...) (__VA_ARGS__)

//     CHAN_MAP(MACRO, (a, b, ..., last))
//
// expands to
//
//     MACRO(a), MACRO(b), ..., MACRO(last)
#define CHAN_MAP1(MACRO, LIST) MACRO(CHAN_FIRST LIST)
#define CHAN_MAP2(MACRO, LIST) \
    MACRO(CHAN_FIRST LIST), CHAN_MAP1(MACRO, CHAN_REST LIST)
#define CHAN_MAP3(MACRO, LIST) \
    MACRO(CHAN_FIRST LIST), CHAN_MAP2(MACRO, CHAN_REST LIST)
#define CHAN_MAP4(MACRO, LIST) \
    MACRO(CHAN_FIRST LIST), CHAN_MAP3(MACRO, CHAN_REST LIST)
#define CHAN_MAP5(MACRO, LIST) \
    MACRO(CHAN_FIRST LIST), CHAN_MAP4(MACRO, CHAN_REST LIST)
#define CHAN_MAP6(MACRO, LIST) \
    MACRO(CHAN_FIRST LIST), CHAN_MAP5(MACRO, CHAN_REST LIST)
#define CHAN_MAP7(MACRO, LIST) \
    MACRO(CHAN_FIRST LIST), CHAN_MAP6(MACRO, CHAN_REST LIST)
#define CHAN_MAP8(MACRO, LIST) \
    MACRO(CHAN_FIRST LIST), CHAN_MAP7(MACRO, CHAN_REST LIST)
#define CHAN_MAP9(MACRO, LIST) \
    MACRO(CHAN_FIRST LIST), CHAN_MAP8(MACRO, CHAN_REST LIST)

#define CHAN_MAP_(N, MACRO, LIST) CHAN_CAT(CHAN_MAP, N)(MACRO, LIST)
#define CHAN_MAP(MACRO, LIST) CHAN_MAP_(CHAN_LENGTH LIST, MACRO, LIST)

//     CHAN_MAPP(MACRO, (a, b, ..., last))
//
// is similar to CHAN_MAP, but is provided as an alternative to work around
// the preprocessor's lack of recursion (can't expand CHAN_MAP within
// CHAN_MAP), and additionally there are no commas between expanded elements.
#define CHAN_MAPP1(MACRO, LIST) MACRO(CHAN_FIRST LIST)
#define CHAN_MAPP2(MACRO, LIST) \
    MACRO(CHAN_FIRST LIST) CHAN_MAPP1(MACRO, CHAN_REST LIST)
#define CHAN_MAPP3(MACRO, LIST) \
    MACRO(CHAN_FIRST LIST) CHAN_MAPP2(MACRO, CHAN_REST LIST)
#define CHAN_MAPP4(MACRO, LIST) \
    MACRO(CHAN_FIRST LIST) CHAN_MAPP3(MACRO, CHAN_REST LIST)
#define CHAN_MAPP5(MACRO, LIST) \
    MACRO(CHAN_FIRST LIST) CHAN_MAPP4(MACRO, CHAN_REST LIST)
#define CHAN_MAPP6(MACRO, LIST) \
    MACRO(CHAN_FIRST LIST) CHAN_MAPP5(MACRO, CHAN_REST LIST)
#define CHAN_MAPP7(MACRO, LIST) \
    MACRO(CHAN_FIRST LIST) CHAN_MAPP6(MACRO, CHAN_REST LIST)
#define CHAN_MAPP8(MACRO, LIST) \
    MACRO(CHAN_FIRST LIST) CHAN_MAPP7(MACRO, CHAN_REST LIST)
#define CHAN_MAPP9(MACRO, LIST) \
    MACRO(CHAN_FIRST LIST) CHAN_MAPP8(MACRO, CHAN_REST LIST)

#define CHAN_MAPP_(N, MACRO, LIST) CHAN_CAT(CHAN_MAPP, N)(MACRO, LIST)
#define CHAN_MAPP(MACRO, LIST) CHAN_MAPP_(CHAN_LENGTH LIST, MACRO, LIST)

#endif

