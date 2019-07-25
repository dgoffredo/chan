#ifndef INCLUDED_CHAN_SELECT_RANDOM
#define INCLUDED_CHAN_SELECT_RANDOM

// This component provides facilities for generating "weak" pseudo-random
// integer sequences, and for applying these sequences to shuffling the
// elements of a sequence.  `chan::select` uses this to break ties.

#include <algorithm>  // where std::swap is in C++98
#include <cassert>
#include <utility>  // where std::swap is in C++11

namespace chan {

// Return the next value in a pseudo-random sequence of 15-bit positive
// integers based on the specified `state`, which will be modified by this
// function.  Subsequently calling `random15` again with the altered state will
// return the next value in the sequence.
inline int random15(int& state) {
    // Use an unsigned integer so we get modular arithmetic (rollover on
    // overflow).
    unsigned nextState = state;

    // See section 1.1 "LCG(2^31; 1103515245; 12345; 12345) ANSIC" of the paper
    // "A collection of selected pseudorandom number generators with linear
    // structures," by Karl Entacher.  That paper is available online, as of
    // this writing, at the following URI:
    // https://pdfs.semanticscholar.org/6ae5/
    //                                 fe88d296e70f188b0d12207d468c8f36e262.pdf
    nextState = nextState * 1103515245 + 12345;

    // The sequence of integers produced by the above LCG is not the same
    // sequence produced by repeated use of this function, so update the state
    // now before we return the actual result.
    state = nextState;

    // Take only the upper 15 bits, but shift them to be the lower bits.  This
    // corrects for the "std::rand() lower bits are not very random" issue.
    const int maskLower15 = 0x7FFF;
    return (nextState >> 16) & maskLower15;
}

// This function-like object produces a pseudo-random sequence of 15-bit
// positive integers based on an initial seed provided on construction.
class Random15 {
    int state;

  public:
    explicit Random15(int seed)
    : state(seed) {
    }

    int operator()() {
        return random15(state);
    }
};

// Return a uniformly random integer derived from a system provided entropy
// source (e.g. "/dev/urandom"), or return zero if no such source is available.
int systemRandom();

// Return an random integer from the specified range `[low, high]` (inclusive
// on both sides), where each integer in the range has an equal probability of
// being returned. Use the specified `generator` to provide a pseudo-random
// sequence of 15-bit positive integers.  The behavior is undefined unless
// `low <= high` and also that `high - low + 1` can be expressed using no more
// than 15 bits (i.e. is less than or equal `2**16 - 1`, or 65535).
inline int randomInt(int low, int high, Random15& getRandom) {
    const int upper = high - low + 1;
    assert(upper >= 1);

    if (upper == 1) {
        return low;  // equals `high`
    }

    // Now we calculate how many bits it takes to express `upper`, and at the
    // same time create a bit mask that will select only that many of the lower
    // bits of an integer.  Then we mask repeated invocations of `getRandom`
    // until we get a number less than `upper`.  Then the answer is that added
    // to `low`.
    int mask = 1;
    int temp = upper;
    while (temp >>= 1) {
        mask = (mask << 1) | 1;
    }

    assert(mask < (1 << 16));

    int candidate;
    do {
        candidate = getRandom() & mask;
    } while (candidate >= upper);

    return low + candidate;
}

template <typename ITERATOR>
void shuffle(ITERATOR begin, ITERATOR end, Random15& generator) {
    // This implementation is based off of the "third version" of the
    // "possible implementation" section of the following cppreference.com
    // article, accessed July 17, 2019:
    // https://en.cppreference.com/w/cpp/algorithm/random_shuffle
    //
    // The idea is to first swap the last element with any other element
    // (including itself).  This fixes the value of the last element.  Then
    // swap the second-to-last element with any but the last element, etc.

    assert(end - begin + 1 < (1 << 16));

    const int n = end - begin;
    for (int i = n - 1; i > 0; --i) {
        using std::swap;
        swap(begin[i], begin[randomInt(0, i, generator)]);
    }
}

template <typename CONTAINER>
void shuffle(CONTAINER& container, Random15& generator) {
    shuffle(container.begin(), container.end(), generator);
}

// Note that for repeated shuffling, it's better to use the overloads of
// `shuffle` that take `Random15&`, so that `systemRandom` need not get called
// as often.
template <typename ITERATOR>
void shuffle(ITERATOR begin, ITERATOR end) {
    Random15 generator(systemRandom());
    shuffle(begin, end, generator);
}

// Note that for repeated shuffling, it's better to use the overloads of
// `shuffle` that take `Random15&`, so that `systemRandom` need not get called
// as often.
template <typename CONTAINER>
void shuffle(CONTAINER& container) {
    shuffle(container.begin(), container.end());
}

}  // namespace chan

#endif
