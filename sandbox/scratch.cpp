#include <chan/chan/chan.h>
#include <chan/debug/trace.h>
#include <chan/errors/error.h>
#include <chan/fileevents/readintobuffer.h>
#include <chan/fileevents/readintostring.h>
#include <chan/select/lasterror.h>
#include <chan/select/random.h>
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
#include <pthread.h>
#include <unistd.h>  // sleep

namespace {

struct Channels {
    chan::Chan<std::string>& input;
    chan::Chan<std::string>& output;

    Channels(chan::Chan<std::string>& input, chan::Chan<std::string>& output)
    : input(input)
    , output(output) {
    }
};

void* sendAndRecvChans(void* channelsRaw) {
    assert(channelsRaw);
    Channels& channels = *static_cast<Channels*>(channelsRaw);

    const char* const words[] = {"foo", "bar", "baz"};
    const int quota = sizeof(words) / sizeof(words[0]);
    int numSent     = 0;
    int numReceived = 0;
    std::string buffer;

    while (numSent < quota && numReceived < quota) {
        switch (chan::select(channels.input.recv(&buffer),
                             channels.output.send(*(words + numSent)))) {
            case 0:
                CHAN_TRACE("I received ", buffer);
                ++numReceived;
                break;
            case 1:
                CHAN_TRACE("I sent ", *(words + numSent));
                ++numSent;
                break;
            default:
                throw chan::lastError();
        }
    }

    while (numSent < quota) {
        channels.output.send(*(words + numSent));
        ++numSent;
    }

    while (numReceived < quota) {
        channels.input.recv(&buffer);
        ++numReceived;
    }

    return 0;
}

int testChanMultiplex(int argc, char* argv[]) {
    chan::Chan<std::string> chan1;
    chan::Chan<std::string> chan2;

    Channels channels1(chan1, chan2);
    Channels channels2(chan2, chan1);
    int      numThreadPairs = 1;

    if (argc > 1) {
        numThreadPairs = std::atoi(argv[1]);
    }

    std::vector<pthread_t> threads1(numThreadPairs);
    for (int i = 0; i < numThreadPairs; ++i) {
        const int rc = pthread_create(&threads1[i], 0, sendAndRecvChans, &channels1);
        assert(rc == 0);
    }

    std::vector<pthread_t> threads2(numThreadPairs);
    for (int i = 0; i < numThreadPairs; ++i) {
        const int rc = pthread_create(&threads2[i], 0, sendAndRecvChans, &channels2);
        assert(rc == 0);
    }

    for (int i = 0; i < numThreadPairs; ++i) {
        int rc = pthread_join(threads1[i], 0);
        assert(rc == 0);
        rc = pthread_join(threads2[i], 0);
        assert(rc == 0);
    }

    return 0;
}

void* sendChan(void* chanPtrRaw) {
    CHAN_TRACE("I was just created.");

    chan::Chan<std::string>* chanPtr =
        static_cast<chan::Chan<std::string>*>(chanPtrRaw);

    assert(chanPtr);

    chan::Chan<std::string>& names   = *chanPtr;
    const char* const        words[] = { "foo", "bar", "hello" };
    const char* const* const end = words + sizeof(words) / sizeof(words[0]);

    for (const char* const* name = words; name != end; ++name) {
        switch (chan::select(names.send(*name))) {
            case 0:
                CHAN_TRACE("()()() I sent: ", *name);
                break;
            default:
                std::cout << "Error sending: " << chan::lastError().what()
                          << std::endl;
        }
    }

    return 0;
}

void* recvChan(void* chanPtrRaw) {
    CHAN_TRACE("I was just created.");

    chan::Chan<std::string>* chanPtr =
        static_cast<chan::Chan<std::string>*>(chanPtrRaw);

    assert(chanPtr);

    chan::Chan<std::string>& names = *chanPtr;
    std::string              msg;

    for (int i = 0; i < 3; ++i) {
        switch (chan::select(names.recv(&msg))) {
            case 0:
                CHAN_TRACE("()()() I received: ", msg);
                break;
            default:
                std::cout << "Error receiving: " << chan::lastError().what()
                          << std::endl;
        }
    }

    return 0;
}

int testChan(int argc, char* argv[]) {
    chan::Chan<std::string> names;
    int                     numThreadPairs = 1;

    if (argc > 1) {
        numThreadPairs = std::atoi(argv[1]);
    }

    std::vector<pthread_t> senders(numThreadPairs);
    for (int i = 0; i < numThreadPairs; ++i) {
        const int rc = pthread_create(&senders[i], 0, sendChan, &names);
        assert(rc == 0);
    }

    std::vector<pthread_t> receivers(numThreadPairs);
    for (int i = 0; i < numThreadPairs; ++i) {
        const int rc = pthread_create(&receivers[i], 0, recvChan, &names);
        assert(rc == 0);
    }

    for (int i = 0; i < numThreadPairs; ++i) {
        int rc = pthread_join(senders[i], 0);
        assert(rc == 0);
        rc = pthread_join(receivers[i], 0);
        assert(rc == 0);
    }

    return 0;
}

int testReadIntoString(int argc, char* argv[]) {
    assert(argc > 1);
    const char* const path = argv[1];

    const int file = ::open(path, O_RDONLY | O_NONBLOCK);

    assert(file != -1);

    std::string destination;
    chan::read(file, destination);
    std::cout << "read the following: " << destination << "\n";

    std::string indirect = chan::read(file, destination);
    std::cout << "Then read this: " << indirect << "\n";

    std::cout << "Finally read this: " << chan::read(file, destination)
              << "\n";

    ::close(file);

    return 0;
}

int testReadIntoBuffer(int argc, char* argv[]) {
    assert(argc > 1);
    const char* const path = argv[1];

    const int file = ::open(path, O_RDONLY | O_NONBLOCK);

    assert(file != -1);

    char buffer[64] = {};
    chan::read(file, buffer);
    std::cout << "read the following:\n";
    std::cout.write(buffer, sizeof(buffer)) << "\n";

    std::cout << "and then read an additional " << chan::read(file, buffer)
              << " bytes.\n";

    ::close(file);

    return 0;
}

int testSelectRead(int argc, char* argv[]) {
    assert(argc > 1);
    const char* const path = argv[1];

    const int file = ::open(path, O_RDONLY | O_NONBLOCK);

    assert(file != -1);

    char buffer[64] = {};

    using chan::read;
    using chan::seconds;
    using chan::select;
    using chan::timeout;

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

    return 0;
}

int testTimeoutAndDeadline(int, char*[]) {
    const chan::TimePoint when = chan::now() + chan::milliseconds(2);
    const int             rc =
        chan::select(chan::deadline(when), chan::deadline(when)
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

    return 0;
}

int testTimePointAndDuration(int, char*[]) {
    const chan::TimePoint before = chan::now();
    std::cout << "We begin " << (before - chan::TimePoint())
              << " since clock start.\n";
    sleep(3);
    std::cout << "We just slept for " << (chan::now() - before) << "\n";

    return 0;
}

int testShuffle(int, char* argv[]) {
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

    return 0;
}

int testRandom(int, char* argv[]) {
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

int main(int argc, char* argv[]) {
    assert(argc > 1);

    const int testNum = std::atoi(argv[1]);
    --argc;
    ++argv;

    switch (testNum) {
        case 0:
            return testChan(argc, argv);
        case 1:
            return testReadIntoString(argc, argv);
        case 2:
            return testReadIntoBuffer(argc, argv);
        case 3:
            return testSelectRead(argc, argv);
        case 4:
            return testTimeoutAndDeadline(argc, argv);
        case 5:
            return testTimePointAndDuration(argc, argv);
        case 6:
            return testShuffle(argc, argv);
        case 7:
            return testRandom(argc, argv);
        case 8:
            return testChanMultiplex(argc, argv);
        default:
            std::cerr << "Invalid test number " << testNum << "\n";
            return 1;
    }
}