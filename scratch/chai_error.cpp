
#include <chai_error.h>

#include <cstring>
#include <sstream>

namespace chai {

std::runtime_error error(int rcode, const char* prefix) {
    std::ostringstream message;
    message << prefix << ": error " << rcode << " " << std::strerror(rcode);
    return std::runtime_error(message.str());
}

}  // namespace chai
