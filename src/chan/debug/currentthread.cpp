#include <chan/debug/currentthread.h>

#include <algorithm>
#include <cassert>
#include <map>
#include <vector>

#include <pthread.h>

namespace chan {
namespace {

std::vector<pthread_t> threads;

class ThreadEquals {
    pthread_t lookingFor;

  public:
    explicit ThreadEquals(pthread_t lookingFor)
    : lookingFor(lookingFor) {
    }

    bool operator()(pthread_t candidate) const {
        return pthread_equal(candidate, lookingFor);
    }
};

}  // namespace

int currentThread() {
    const pthread_t                              current = pthread_self();
    const std::vector<pthread_t>::const_iterator found =
        std::find_if(threads.begin(), threads.end(), ThreadEquals(current));
    if (found != threads.end()) {
        // Found it.  Return its index in `threads`.
        return found - threads.begin() + 1;
    }
    else {
        // Haven't seen this thread before.  Add it to `threads` and return
        // its index in `threads`.
        threads.push_back(current);
        return threads.size();
    }
}

}  // namespace chan
