#ifndef INCLUDED_CHAN_CHANEVENTS_CHANPROTOCOL
#define INCLUDED_CHAN_CHANEVENTS_CHANPROTOCOL

namespace chan {

class ChanProtocolMessage {
  public:
    enum Value {
        HI,      // visitor greets a sitter
        READY,   // sitter acknowledges visitor and awaits transfer
        DONE,    // visitor notifies sitter that transfer succeeded
        CANCEL,  // either notifies the other that it cancelled
        ERROR,   // visitor tells the sitter that the transfer failed
        POKE     // tell a sitter to become a visitor
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
