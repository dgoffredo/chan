#ifndef INCLUDED_CHAN_FILES_PIPEPAIR
#define INCLUDED_CHAN_FILES_PIPEPAIR

namespace chan {

struct PipePair {
    // file descriptors
    int fromVisitor;
    int toSitter;
    int fromSitter;
    int toVisitor;

    // reference counting (not a file descriptor)
    int referenceCount;
};

}  // namespace chan

#endif
