#include <chan_select.h>

#include <poll.h>

namespace chan {
namespace {

// A `Selector` is the object that holds all of the state during a call to
// `chan::select`.
class Selector {
    chai::EventRef*       eventsBegin;
    const chai::EventRef* eventsEnd;
    std::vector<pollfd>   pollfds;  // timeouts get a `-1` file descriptor

  public:
    explicit Selector(chai::EventRef* events, const chai::EventRef* end);

    IoEvent handleIoEvent(const IoEvent& fileEvent, unsigned eventIndex);

    int operator()();
};

Selector::Selector(chai::EventRef* events, const chai::EventRef* end)
    : eventsBegin(events), eventsEnd(end), pollfds(end - events) {}

int Selector::operator()() {
    // TODO:
    // - create poll set
    // - deduce timeout (if applicable)
    // - poll()
    //     - error? If it's a real error, throw or return. Otherwise, continue.
    //     - events? Call corresponding handlers.
    //     - randomize? Yes...
}

IoEvent Selector::handleIoEvent(const IoEvent& fileEvent,
                                unsigned       eventIndex) {
    // TODO
    // fulfill..., cancel?
}

}  // namespace

namespace detail {

int selectImpl(chai::EventRef* eventsBegin, const chai::EventRef* eventsEnd) {
    return Selector(eventsBegin, eventsEnd)();
}

}  // namespace detail
}  // namespace chan