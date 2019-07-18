#include "random.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <vector>

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

int main(int argc, char *argv[]) {
    return testShuffle(argc, argv);
}