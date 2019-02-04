#ifndef INCLUDED_CHAI_ERROR
#define INCLUDED_CHAI_ERROR

#include <stdexcept>

namespace chai {

std::runtime_error error(int rcode, const char* prefix);
// Return an exception whose 'what()' returns a message beginning with the
// specified 'prefix' and ending with the numeric value and name of the
// specified 'errno'-compatible 'rcode'.  Note that the result of this
// function is intended to be used in a 'throw' statement, e.g.
//..
// if (somePosixCall() == -1) {
//     throw error(errno, "Something bad happened, m'kay?");
// }
//..

}  // namespace chai

#endif