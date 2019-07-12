#ifndef INCLUDED_CHAN_FILES_PIPEPAIR
#define INCLUDED_CHAN_FILES_PIPEPAIR

namespace chan {

struct PipePair {
    int fromVisitor;
    int toSitter;
    int fromSitter;
    int toVisitor;
};

}  // namespace chan

#endif
