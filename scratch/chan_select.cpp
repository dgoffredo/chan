#include <chan/select/select.h>

#include <poll.h>

namespace chan {
namespace {

// A `Selector` is the object that holds all of the state during a call to
// `chan::select`.
class Selector {
    EventRef* const events;
    const int             numEvents;
    std::vector<pollfd>   pollfds;

  public:
    explicit Selector(EventRef* events, const EventRef* end);

    IoEvent handleIoEvent(const IoEvent& fileEvent, unsigned eventIndex);  // ?

    int operator()();
};

Selector::Selector(EventRef* events, const EventRef* end)
    : events(events), numEvents(end - events), pollfds(numEvents) {}

int Selector::operator()() try {
    // TODO:
    // - create poll set
    // - deduce timeout (if applicable)
    // - poll()
    //     - error? If it's a real error, throw or return. Otherwise, continue.
    //     - events? Call corresponding handlers.
    //     - randomize? Yes...
}
catch (const Error& error) {
    // TODO
}
catch (const std::exception& error) {
    // TODO
}
catch (...) {
    // TODO
}

IoEvent Selector::handleIoEvent(const IoEvent& fileEvent,
                                unsigned       eventIndex) {
    // TODO
    // fulfill..., cancel?
}

}  // namespace

int selectImpl(EventRef* eventsBegin, const EventRef* eventsEnd) {
    return Selector(eventsBegin, eventsEnd)();
}

}  // namespace chan
