#include <chan/fileevents/readintobuffer.h>
#include <chan/select/random.h>
#include <chan/errors/error.h>
#include <chan/select/lasterror.h>
#include <chan/select/select.h>
#include <chan/time/duration.h>
#include <chan/time/timepoint.h>
#include <chan/timeevents/deadline.h>
#include <chan/timeevents/timeout.h>

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <fcntl.h>  // open
#include <unistd.h>  // sleep

namespace {

int testRead(int argc, char *argv[]) {
    assert(argc > 1);
    const char* const path = argv[1];

    const int file = ::open(path, O_RDONLY | O_NONBLOCK);

    assert(file != -1);

    char buffer[64] = {};

    using chan::select; using chan::read; using chan::timeout; using chan::seconds;

    switch (select(read(file, buffer), timeout(seconds(5)))) {
        case 0:
            std::cout << "read the following:\n" << buffer << "\n";
            break;
        case 1:
            std::cout << "timed out\n";
            break;
        default:
            std::cout << "error: " << chan::lastError().what() << "\n";
    }

    ::close(file);
}

void testTimeoutAndDeadline(int, char*[]) {
    const chan::TimePoint when = chan::now() + chan::milliseconds(2);
    const int rc = chan::select(  chan::deadline(when)
                                , chan::deadline(when)
                                // , chan::timeout(chan::milliseconds(498))
                                // , chan::deadline(chan::now() + chan::nanoseconds(24))
                                // , chan::timeout(chan::milliseconds(499))
                                // , chan::timeout(chan::milliseconds(498))
                                );
    if (rc < 0) {
        std::cout << "An error occurred: " << chan::lastError() << "\n";
    }
    else {
        std::cout << "Event " << rc << " happened first.\n";
    }
}

void testTimePointAndDuration(int, char*[]) {
    const chan::TimePoint before = chan::now();
    std::cout << "We begin " << (before - chan::TimePoint())
              << " since clock start.\n";
    sleep(3);
    std::cout << "We just slept for " << (chan::now() - before) << "\n";
}

int testShuffle(int argc, char *argv[]) {
    const int length = std::atoi(argv[1]);

    std::vector<int> numbers;
    numbers.reserve(length);

    for (int i = 0; i < length; ++i) {
        numbers.push_back(i);
        std::cout << " " << i;
    }
    std::cout << "\n";

    std::string    buffer;
    chan::Random15 generator(0);  // seeded with zero

    while (std::getline(std::cin, buffer)) {
        chan::shuffle(numbers, generator);

        for (int i = 0; i < length; ++i) {
            std::cout << " " << numbers[i];
        }
        std::cout << "\n";
    }
}

int testRandom(int argc, char *argv[]) {
    const int low    = std::atoi(argv[1]);
    const int high   = std::atoi(argv[2]);
    const int trials = std::atoi(argv[3]);

    std::map<int, int> counts;
    chan::Random15     generator(0);  // seeded with zero
    for (int i = trials; i; --i) {
        ++counts[chan::randomInt(low, high, generator)];
    }

    for (std::map<int, int>::const_iterator it = counts.begin();
         it != counts.end();
         ++it) {
        const double expected      = double(trials) / (high - low + 1);
        const int    actual        = it->second;
        const double relativeError = (expected - actual) / expected;

        // std::cout << it->first << " " << actual << " # " << relativeError
        //           << "\n";
        std::cout << it->first << " " << relativeError << "\n";
    }

    std::cout << "# trials = " << trials
              << "    1/sqrt(trials) = " << (1.0 / std::sqrt(trials)) << "\n";

    return 0;
}

}  // namespace

int main(int argc, char *argv[]) {
    return testRead(argc, argv);
}