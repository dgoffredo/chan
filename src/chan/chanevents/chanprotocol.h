#ifndef INCLUDED_CHAN_CHANEVENTS_CHANPROTOCOL
#define INCLUDED_CHAN_CHANEVENTS_CHANPROTOCOL

namespace chan {

class ChanProtocolMessage {
  public:
    enum Value {
        DONE,   // visitor notifies sitter that transfer succeeded
        ERROR,  // visitor tells the sitter that the transfer failed
        POKE    // tell a sitter to become a visitor
    };

  private:
    Value value;

  public:
    ChanProtocolMessage(Value value)
    : value(value) {
    }

    operator Value() const {
        return value;
    }
};

// Write the specified `message` to the specified `file`.  If an error occurs,
// throw an exception.
void writeMessage(int file, ChanProtocolMessage message);

// Return a `ChanProtocolMessage` read from the specified `file`.  If an error
// occurs, throw an exception.
ChanProtocolMessage readMessage(int file);

}  // namespace chan

#endif
