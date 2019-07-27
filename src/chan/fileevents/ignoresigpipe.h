#ifndef INCLUDED_CHAN_FILEEVENTS_IGNORESIGPIPE
#define INCLUDED_CHAN_FILEEVENTS_IGNORESIGPIPE

// This component exists solely to set the current process to ignore `SIGPIPE`.
// Any program that has this object linked into it will call the relevant
// signal management API during static initialization.

namespace chan {

extern void* sigpipeIgnorer;

}  // namespace chan

#endif
