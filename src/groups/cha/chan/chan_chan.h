#ifndef INCLUDED_CHAN_CHAN
#define INCLUDED_CHAN_CHAN

// TODO doc doc doc doc

#include <bdls_filesystemutil.h>

#include <bslmf_movableref.h>

#include <bslmt_lockguard.h>
#include <bslmt_mutex.h>

#include <bsls_assert.h>

#include <bsl_memory.h>

namespace BloombergLP {
namespace bslma {
class Allocator;
}
}

namespace chan {
using namespace BloombergLP;

///Channels
///--------
// A channel is a rendezvous point between two threads that wish to exchange an
// object. One thread, the sender, calls 'send' on the channel, and another
// thread, the receiver, calls 'recv' on the channel. Senders and receivers
// wait until they can be paired up. Once paired up, the object sent by the
// sender is copied or moved into the storage indicated by the receiver.
//
// This library implements channels as instances of a class template, 'Chan':
//..
//  template <typename OBJECT>
//  class Chan {
//    public:
//      SendEvent send(const OBJECT&);
//      SendEvent send(bslmf::MovableReference<OBJECT>);
//          // Send an object. The destructor of the returned 'SendEvent' will
//          // block until the send is successful. Alternatively, the
//          // 'SendEvent' can be used as an argument to 'select'.
//  
//      RecvEvent recv(OBJECT&);
//      OBJECT    recv();
//          // Receive an object. The destructor of the returned 'RecvEvent'
//          // will block until the receive is successful. Alternatively, the
//          // 'RecvEvent' can be used as an argument to 'select'. The second
//          // overload returns an 'OBJECT' directly, and cannot be used in
//          // 'select'. It's equivalent to the following:
//          //..
//          //  template <typename OBJECT>
//          //  OBJECT recvByValue(Chan<OBJECT>& channel)
//          //  {
//          //      OBJECT result;
//          //      channel.recv(result);
//          //      return result;
//          //  }
//          //..
//  };
//..
// Note that, unlike Go channels, a 'Chan' has no concept of being "closed."
// Information about whether further values will be sent on a 'Chan' must be
// communicated in the messages themselves (e.g. using a 'bdlb::Variant') or
// out of line (e.g. on another 'Chan' dedicated for this purpose).
//
///The Event Concept
///-----------------
// 'chan::select' fulfills exactly one from a set of "events," its function
// arguments. An event is any object that satisfies the event concept. The
// event concept is a set of member functions.
//
// The default versions of the event methods assume that file descriptors will
// be 'read' from:
//..
//  int file();
//      // Return a file descriptor that can be read from as a synchronization
//      // point that indicates when the event is ready to be fulfilled.
//
//  int fulfill(int fileDescriptor);
//      // Complete the event that began by reading from the specified
//      // 'fileDescriptor'. Return '-1' on success. Return a file descriptor
//      // to replace 'file()' if the event could not be fulfilled.
//
//  void cancel(int fileDescriptor);
//      // Cancel the event that began by reading from the specified
//      // 'fileDescriptor'.
//..
// 'select' will alternatively use extended versions of 'file', 'fulfill', and
// 'cancel' that allow for waiting on file operations other than 'read' (i.e.
// 'write', 'accept', or 'connect').
//..
//  bsl::pair<int, btlso::EventType::Type> file();
//      // Return a file descriptor on which to wait for the returned file
//      // operation, which indicates when the event is ready to be fulfilled.
//
//  bsl::pair<int, btlso::EventType::Type> fulfill(
//          int                    fileDescriptor,
//          btlso::EventType::Type operation);
//      // Complete the event that began by waiting for the specified
//      // 'operation' on the specified 'fileDescriptor'. Return a value whose
//      // 'int' component is '-1' to indicate success. Return a file
//      // descriptor and operation to replace 'file()' if the event could not
//      // be fulfilled.
//
//  void cancel(int                    fileDescriptor,
//              btlso::EventType::Type operation);
//      // Cancel the event that most recently waited for the specified
//      // 'operation' on the specified 'fileDescriptor'.
//..
// Finally, "timeout" events have the following form:
//..
//  bsls::TimeInterval file();
//      // Return a relative timeout after which this even might be considered
//      // fulfilled.
//
//  bsls::TimeInterval fulfill(bsls::TimeInterval timeout);
//      // Complete the event that began by waiting for the specified
//      // 'timeout'. Return a default constructed 'bsls::TimeInterval' on
//      // success (the default constructed value represents an empty interval)
//      // or return a value to replace 'file()' if this event was not
//      // fulfilled.
//
//  void cancel(bsls::TimeInterval timeout);
//      // Cancel the event that most recently waited for the specified
//      // relative 'timeout'.
//..
// 'select' will alternatively use extended versions of 'file', 'fulfill', and
// 'cancel' that allow for waiting on file operations other than 'read':
//..
//  bsl::pair<int, btlso::EventType::Type> file();
//      // Return a file descriptor on which to wait for the returned file
//      // operation, which indicates when the event is ready to be fulfilled
//      // or canceled.
//
//..
// Here's an example of the simplest possible non-trivial event -- reading from
// an open file:
//..
//  class FileRead {
//      // This 'class' satisfies the "event" concept, and is intended to be
//      // used in 'select' to possibly read data from a file.
//  
//      // DATA
//      int          d_fileDescriptor;
//      char        *d_buffer_p;
//      bsl::size_t  d_bufferSize;
//      bsl::size_t  d_numBytesRead;
//      
//    public:
//      // CREATORS
//      FileRead(int file, char *buffer, bsl::size_t size)
//      : d_fileDescriptor(file)
//      , d_buffer_p(buffer)
//      , d_bufferSize(size)
//      , d_numBytesRead()
//      {}
//  
//      int file() const {
//          return d_fileDescriptor;
//      }
//  
//      int fulfill(int file) const {
//          BSLS_ASSERT_OPT(file == d_fileDescriptor);
//  
//          const int rc = ::read(file, d_buffer_p, d_bufferSize);
//          if (rc < 0) {
//              // Error. Return the same file for the next try. Note that this
//              // means passing an invalid file would cause CPU spin.
//              return file;                                          // RETURN
//          }
//  
//          // The read was successful.
//          d_numBytesRead = rc;
//          return -1;
//      }
//  
//      void cancel(int) const {
//          // nothing to do
//      }
//  
//      bsl::size_t numBytesRead() const {
//          return d_numBytesRead();
//      }
//  };
//  
//  FileRead read(int file, char *buffer, bsl::size_t size) {
//      return FileRead(file, buffer, size);
//  }
//..
// Now 'read(file, buffer, size)' can be passed as an argument to 'select', and
// if the read into 'buffer' successfully happens, 'select' will return the
// zero-based index of the argument the call to 'read' occupied.
//
// Notice that if the intention were to select the *availability* of reading
// from the file, then 'fulfill' could just always return success.
//
///How 'select' Works
///------------------
// 'select' begins by calling 'file()' on all of its arguments to create a poll
// set. It then blocks on its event manager's dispatching function, waiting for
// one of the file/operation combinations returned by the calls to 'file()' to
// occur. When one occurs, 'select' calls 'fulfill' on the corresponding
// argument. If 'fulfill' indicates success, then 'select' calls 'cancel' on
// all of the other arguments, and returns the zero-based index of whichever
// argument was fulfilled. If 'fulfill' does not indicate success, then
// 'select' replaces the file/operation combination for that argument based on
// the return value of 'fulfill', and re-enters its event manager's dispatching
// function. This loop continues indefinitely until an event is fulfilled.
//
///Optional Special Syntax
///-----------------------
// More for fun than any good reason, this library defines special identifiers
// and operator overloads in the namespace 'chan::syntax' that allow Go-like
// send and receive syntax to be used in 'select' calls:
//..
//  using namespace chan::syntax;
//  using chan::select;
//
//  switch (select(_ <-ch1, ch2 <- "hello", ch3 <- ch4)) {
//    case 0: std::cout << "Received something from ch1.\n";             break;
//    case 1: std::cout << "sent \"hello\" into ch2.\n";                 break;
//    case 2: std::cout << "either sent ch4 into ch3 or received from "
//                         "ch4, depending on the types.\n"              break;
//  }
//..
// 'namespace chan::syntax' defines 'operator<' and 'operator-' templates,
// along with a special identifier '_', to simulate Go's send and receive
// syntax. The '_' identifier exists to indicate that the specified operation
// is to be waited upon but not performed. There is apparent ambiguity in C++'s
// realization of the receive syntax, since the token sequence '= <-' is not
// valid in C++. Nonetheless, there is no ambiguity, due to the type system:
//..
//  1.  _ <-channel           is the same as  channel.recv(/*ignored*/)
//  3.  value <- channel      is the same as  channel.recv(value)
//  4.  channel <- value      is the same as  channel.send(value)
//  5.  channel1 <- channel2  either channel2.recv(channel1) or
//                            channel1.send(channel2), depending on
//                            the types of channel1 and channel2
//..
// This is not as elegant as Go's syntax, but it's not a big deal since
// whenver channels of channels are used, it is clear which channel is the
// value and which is the communication mechanism.

                            // ==========
                            // class Chan
                            // ==========

template <typename OBJECT = void>
class Chan;  // generic implementation for objects of type 'OBJECT'

template <>
class Chan<void>;  // specific implementation for file descriptors

template <typename OBJECT>
class Chan {
    // TODO

    // DATA
    bsl::shared_ptr<chai::ChanState<OBJECT> > d_state_sp;

  public:
    // CREATORS
    explicit Chan(bslma::Allocator *allocator = 0);
        // Create a 'Chan' object. Optionally specify an 'allocator' used to
        // supply memory. If 'allocator' is zero, then the currently installed
        // default allocator is used.

    // MANIPULATORS
    SendEvent<OBJECT> send(const OBJECT&                   object);
    SendEvent<OBJECT> send(bslmf::MovableReference<OBJECT> object);
        // TODO

    RecvEvent recv(OBJECT& destination);
    OBJECT    recv();
        // TODO
};

// TODO

// ============================================================================
//                          INLINE DEFINITIONS
// ============================================================================

                        // -------------------
                        // class SendOperation
                        // -------------------

// CREATORS
template <typename OBJECT>
SendOperation<OBJECT>::SendOperation(
    const bsl::shared_ptr<chai::ChanState<OBJECT> >& state)
: d_state_sp(state)
{
}

// MANIPULATORS
template <typename OBJECT>
int SendOperation<OBJECT>::file()
{
    BSLS_ASSERT_OPT(d_state_sp);

    chai::ChanState<OBJECT>& state = *d_state_sp;

    bsl::list<Sender<OBJECT> > meList(state.d_allocator_p);
    me.emplace_back();  // The default constructor is fine (nothing to send).
    const Sender<OBJECT>& me = meList.front();

    bslmt::LockGuard<bslmt::Mutex> lock(state.d_mutex);                 // LOCK

    // There are only two cases. Either
    //     1. I'm the only sender and there are receivers,
    // or
    //     2. Other.
    // For case (2), I create a 'Sender<OBJECT>' for myself, enqueue it onto
    // the senders queue, and return the read end of the associated pipe.
    // For case (1), I additionally will try to perform a send. The file I
    // return will be the "from receiver" end of the receiver's pipe, after
    // I've said 'k_HAI'.
    // TODO: No. I'll get a pipe, send the write end, and return the read end.
    
    // In either case, I first add myself to the end of the senders queue.
    state.d_senders.splice(state.d_senders.end(), meList);

    // case 1
    if (state.d_senders.size() == 1 && !state.d_receivers.empty())
    {
        Receiver<OBJECT>& receiver = state.d_receivers.front();
        lock.release()->unlock();                                     // UNLOCK

        const chai::Protocol::Message& msg = chai::Protocol::k_HAI;

        // Keep in mind that this writing end of the pipe is non-blocking.
        const int written = 
            bdls::FilesystemUtil::write(receiver.to, msg, sizeof msg);

        if (written != sizeof msg) {
            // TODO: clean up, poke, etc.
            return bdls::FilesystemUtil::k_INVALID_FD;                // RETURN
        }

        return receiver.from                                          // RETURN
    }

    // case 2
    return me.to;
}

template <typename OBJECT>
void SendOperation<OBJECT>::fulfill(int fileDescriptor)
{
    // Since we represent only the *capability* of sending, to fulfill us is
    // not to send anything, but just to clean up.
    cancel(fileDescriptor);
}

template <typename OBJECT>
void SendOperation<OBJECT>::cancel(int fileDescriptor)
{
    // TODO:
    // Remove ourselves from the front of the senders queue.
    //
    // If 'fileDescriptor' is the '.to' of our 'Sender', then we'll communicate
    // lack of interest when the pipe is closed in the 'Sender' destructor.
    //
    // If, alternatively, 'fileDescriptor' is the '.from' of the front
    // 'Receiver', then communicate lack of interest by writing
    // 'chai::Protocol::k_CANCEL' to the '.to' of the 'Receiver'.
    //
    // In either case, 
}

    SendEvent<OBJECT> operator()(const OBJECT& object);
        // Return an event whose fulfillment sends a copy of the specified
        // 'object' through the channel associated with this object.

    SendEvent<OBJECT> operator()(bslmf:MovableRef<OBJECT> object);
        // Return an event whose fulfillment moves the specified 'object'
        // through the channel associated with this object.
};

}  // close package namespace

#endif
