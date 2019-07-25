#ifndef INCLUDED_CHAN_FILES_PIPE
#define INCLUDED_CHAN_FILES_PIPE

namespace chan {

struct Pipe {
    // file descriptors
    int fromVisitor;  // the reading end
    int toSitter;     // the writing end

    // reference counting (not a file descriptor)
    int referenceCount;
};

}  // namespace chan

#endif
