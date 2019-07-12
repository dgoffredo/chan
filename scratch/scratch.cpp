
#include <chai_pipepool.h>

#include <chap_macros.h>

#include <cassert>
#include <iostream>

namespace chai {

std::ostream& operator<<(std::ostream& stream, const PipePair& pipes) {
    return stream << "(" << pipes.fromVisitor << ", " << pipes.toSitter << ", "
                  << pipes.fromSitter << ", " << pipes.toVisitor << ")";
}

}  // namespace chai

int main(int argc, char* argv[]) {
    assert(argc == 2);

    using namespace chai;

    switch (std::atoi(argv[1])) {
        case 1: {
            {
                PipePool* poolPtr = new PipePool;
                PipePool& pool    = *poolPtr;
                PipePair  pipes   = pool.take();

                std::cout << pipes << '\n';
            }
            {
                PipePool pool;
                PipePair pipes = pool.take();

                std::cout << "1: " << pipes << '\n';

                pool.giveBack(pipes);

                PipePair pipes2 = pool.take();
                std::cout << "2: " << pipes2 << '\n';

                PipePair pipes3 = pool.take();
                std::cout << "3: " << pipes3 << '\n';

                pool.giveBack(pipes3, PipePool::e_VISITOR);

                PipePair pipes4 = pool.take();
                std::cout << "4: " << pipes4 << '\n';

                pool.giveBack(pipes3, PipePool::e_SITTER);

                PipePair pipes5 = pool.take();
                std::cout << "5: " << pipes5 << '\n';
            }
        }  // end case 1
        case 2: {
            // TODO
        }  // end case 2
    }      // end switch
}
