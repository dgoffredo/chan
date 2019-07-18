#ifndef INCLUDED_CHAN_DEBUG_CURRENTTHREAD
#define INCLUDED_CHAN_DEBUG_CURRENTTHREAD

namespace chan {

// Return a one-based thread ID identifying the current thread.  The IDs are
// determined by the order in which this function is called for the first time
// on each thread.  The behavior is undefined if two or more threads call
// `currentThread` concurrently (so use some `Mutex`).
int currentThread();

}  // namespace chan

#endif
