#ifndef INCLUDED_CHAN_DEBUG_TRACE
#define INCLUDED_CHAN_DEBUG_TRACE

#include <chan/macros/macros.h>

#ifdef CHAN_TRACE_ON

#include <chan/debug/currentthread.h>
#include <chan/threading/lockguard.h>
#include <chan/threading/mutex.h>

#include <iostream>
#include <sstream>

namespace chan {

extern Mutex traceMutex;

#define CHAN_TRACE_STREAM cHaNtRaCeStReAm

#define CHAN_TRACE_STREAMIFY(TERM) << (TERM)

#define CHAN_TRACE(...)                                                    \
    do {                                                                   \
        std::ostringstream CHAN_TRACE_STREAM;                              \
        CHAN_TRACE_STREAM  CHAN_MAPP(CHAN_TRACE_STREAMIFY, (__VA_ARGS__)); \
        chan::LockGuard    lock(chan::traceMutex);                         \
        std::cerr << "(thread " << chan::currentThread() << ") ";          \
        std::cerr << CHAN_TRACE_STREAM.str() << std::endl;                 \
    } while (false)

}  // namespace chan

#else  // #ifdef CHAN_TRACE_ON

#define CHAN_TRACE_UNUSEDIFY(TERM) (void)sizeof(TERM);

#define CHAN_TRACE(...)                                \
    do {                                               \
        CHAN_MAPP(CHAN_TRACE_UNUSEDIFY, (__VA_ARGS__)) \
    } while (false)

#endif  // #ifdef CHAN_TRACE_ON

#endif  // inclusion guard
