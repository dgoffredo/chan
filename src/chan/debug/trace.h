#ifndef INCLUDED_CHAN_DEBUG_TRACE
#define INCLUDED_CHAN_DEBUG_TRACE

#ifdef CHAN_TRACE_ON

#include <chan/debug/currentthread.h>
#include <chan/macros/macros.h>
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

#define CHAN_TRACE(...) \
    do {                \
    } while (false)

#endif  // #ifdef CHAN_TRACE_ON

#endif  // inclusion guard
