#ifndef INCLUDED_CHAN_MACROS_MACROS
#define INCLUDED_CHAN_MACROS_MACROS

// This header contains preprocessor macros used to generate code elsewhere in
// this library. The maximum argument list length supported by these macros is
// currently nine.
#define CHAN_MAX_ARITY 9

//     CHAN_LENGTH(1, 2, 3, ..., N)
//
// expands to
//
//     N
#define CHAN_LENGTH_(A9, A8, A7, A6, A5, A4, A3, A2, A1, LENGTH, ...) LENGTH
#define CHAN_LENGTH(...) \
    CHAN_LENGTH_(__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, dummy)

//     CHAN_CAT(LEFT, RIGHT)
//
// expands to
//
//    LEFTRIGHT
//
// However, if either of LEFT or RIGHT is a macro invocation, then it will be
// expanded before pasting.  CHAN_CAT can also take more than two arguments.
#define CHAN_CAT_RAW(LEFT, RIGHT) LEFT##RIGHT
#define CHAN_CAT2(HEAD, ...)      CHAN_CAT_RAW(HEAD, __VA_ARGS__)
#define CHAN_CAT3(HEAD, ...)      CHAN_CAT2(HEAD, CHAN_CAT2(__VA_ARGS__))
#define CHAN_CAT4(HEAD, ...)      CHAN_CAT2(HEAD, CHAN_CAT3(__VA_ARGS__))
#define CHAN_CAT5(HEAD, ...)      CHAN_CAT2(HEAD, CHAN_CAT4(__VA_ARGS__))
#define CHAN_CAT6(HEAD, ...)      CHAN_CAT2(HEAD, CHAN_CAT5(__VA_ARGS__))
#define CHAN_CAT7(HEAD, ...)      CHAN_CAT2(HEAD, CHAN_CAT6(__VA_ARGS__))
#define CHAN_CAT8(HEAD, ...)      CHAN_CAT2(HEAD, CHAN_CAT7(__VA_ARGS__))
#define CHAN_CAT9(HEAD, ...)      CHAN_CAT2(HEAD, CHAN_CAT8(__VA_ARGS__))

#define CHAN_CAT_(N, ...) CHAN_CAT_RAW(CHAN_CAT, N)(__VA_ARGS__)
#define CHAN_CAT(...)     CHAN_CAT_(CHAN_LENGTH(__VA_ARGS__), __VA_ARGS__)

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

#define CHAN_FIRST_N(FIRST, ...) FIRST
#define CHAN_FIRST9              CHAN_FIRST_N
#define CHAN_FIRST8              CHAN_FIRST_N
#define CHAN_FIRST7              CHAN_FIRST_N
#define CHAN_FIRST6              CHAN_FIRST_N
#define CHAN_FIRST5              CHAN_FIRST_N
#define CHAN_FIRST4              CHAN_FIRST_N
#define CHAN_FIRST3              CHAN_FIRST_N
#define CHAN_FIRST2              CHAN_FIRST_N
#define CHAN_FIRST1(FIRST)       FIRST
#define CHAN_FIRST_(N, ...)      CHAN_CAT(CHAN_FIRST, N)(__VA_ARGS__)
#define CHAN_FIRST(...)          CHAN_FIRST_(CHAN_LENGTH(__VA_ARGS__), __VA_ARGS__)

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
#define CHAN_MAP(MACRO, LIST)     CHAN_MAP_(CHAN_LENGTH LIST, MACRO, LIST)

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
#define CHAN_MAPP(MACRO, LIST)     CHAN_MAPP_(CHAN_LENGTH LIST, MACRO, LIST)

#endif
