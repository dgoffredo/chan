#ifndef INCLUDED_CHAI_PIPEPAIR
#define INCLUDED_CHAI_PIPEPAIR

namespace chai {

struct PipePair {
    int fromVisitor;
    int toSitter;
    int fromSitter;
    int toVisitor;
};

}  // namespace chai

#endif
