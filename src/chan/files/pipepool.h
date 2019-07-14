#ifndef INCLUDED_CHAN_FILES_PIPEPOOL
#define INCLUDED_CHAN_FILES_PIPEPOOL

#include <chan/files/pipepair.h>
#include <chan/threading/mutex.h>

#include <vector>

namespace chan {

class PipePool {
  public:
    // "Whose" in the sense of "who reads or writes to these descriptors?"
    // So, "toVisitor" and "fromVisitor" belong to `SITTER` (*not*
    // `VISITOR`), because it is the sitter that will be writing to and
    // reading from the visitor.  Similarly, "toSitter" and "fromSitter"
    // belong to `VISITOR`.  `BOTH` indicates all four file
    // descriptors.
    enum Whose { SITTER, VISITOR, BOTH };

  private:
    Mutex                 d_mutex;
    std::vector<PipePair> d_pipes;

    PipePool(const PipePool&) /* = delete */;

  public:
    ~PipePool();

    PipePair take();

    void giveBack(const PipePair& pipes, Whose whose = BOTH);
};

}  // namespace chan

#endif
