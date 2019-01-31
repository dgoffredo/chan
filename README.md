`cpp-channels`
==============
Go-like Channels in C++03

Why
===
I want to be able to `select` to send and receive both C++ objects between
threads and to send and receive bytes to and from files (pipes, sockets, etc.),
or both.

Currently the only option seems to be to drop down to C and do a `poll` dance.
So, this library wraps that up into a convenient abstraction: `class Chan`.

What
====
`cpp-channels` is a C++03 library in the form of a [BDE][BDE]-style package
group, `cha`, whose main elements are:

- `chan::Chan`: a class template representing either an unbuffered channel
  of objects of a particular type, or a handle to a file (pipe, socket, etc.)
- `chan::make`: a function for making `Chan`nels.
- `chan::select`: a function for performing exactly one operation from a set of
  sends and/or received on `Chan`nels.
- `chan::delayMs`: a function that returns a receive operation on a `Chan`nel
  that is readable after a specified number of milliseconds.
- `chan::deadline`: a function that returns a read operation on a `Chan`el that
  is readable after a specified point in time.

How
===
Here's a program that reads `StockTick` objects from a library, publishes
metrics every two seconds, and exits when any standard input is entered:
```C++
#include <chan_chan.h>
#include <chan_select.h>
#include <stock_exchange.h>
#include <iostream>

chan::Chan<StockTick> subscribe();                         // defined elsewhere
void                  processStockTick(const StockTick&);  // defined elsewhere
void                  publishMetrics();                    // defined elsewhere

int main(int argc, char *argv[])
{
    using chan::select;
    using chan::make; 
    using chan::delayMs;

    chan::Chan<>          input = make(0);  // from standard input
    chan::Chan<StockTick> ticks = subscribe();
    StockTick             tick;

    for (;;) {
        switch(select(input.recv, ticks.recv(&tick), delayMs(2000))) {
          case 0: {
            std::cout << "Exiting due to caller input.\n";
            return 0;
          }
          case 1:
            processStockTick(tick);
            break;
          default:
            publishMetrics();
        }
    }
}
```

Here's a function, meant to be run on its own thread, that reads lines from an
input file/pipe/socket and then sends them as `std::string` objects on a
`Chan`nel, but quits if there's ever input on a "done" `Chan`nel:
```C++
void lines(int                     file,  // non-blocking
           chan::Chan<std::string> output,
           chan::Chan<bool>        done,
           std::string             delimiter = "\n")
{
    std::vector        buffer(2 * 1024 * 1024);
    char        *const buf = buffer.data();
    std::string        current;
    chan::Chan<>       input(file);

    for (;;) {
        switch (chan::select(input.recv(buf, buffer.size()), done.recv)) {
          case 0: {
              current.append(buf, input.count()));
              std::size_t pos = 0;
              for (;;) {
                  const std::size_t delim = current.find(delimiter, pos);
                  if (delim == std::string::npos)
                      break;
                  if (chan::select(output.send(current.substr(pos, delim)),
                                   done.recv) == 1)
                      return;                                         // RETURN
                  pos = newline + delimiter.size();
              }
              current.erase(0, pos);
          } break;
          case 1:
              return;                                                 // RETURN
        }
    }
}
```

Finally, here's a plain old job-consuming worker; no `select` required:
```C++
void worker(chan::Chan<std::function<Result()> jobs,
            chan::Chan<Result>                 results)
{
    for (;;) {
        const std::function<Result()> job = jobs.recv();

        if (!job)
            // an unset function object indicates "quit"
            return;                                                   // RETURN

        results.send(job());
    }
}
```

Keep in mind that since `Chan`nels are not buffered, the last example is not
the same thing and consuming from and producing to queues, but instead only
allows the computing power at a particular stage of a data pipeline to be
increased (by using more workers).

More
----
### Proposed Header File
Here is a more techincal description of the capbilities I'd like this library
to offer.
```C++
namespace chan {
                            // ==========
                            // class Chan
                            // ==========

template <typename OBJECT = void>
class Chan {
    // Generic implementation of 'Chan': an unbuffered channel of objects of
    // type 'OBJECT'.

  public:
    // MANIPULATORS
    void close();

    const Send<OBJECT> send;
    const Recv<OBJECT> recv;

    // The two above callable 'const' data member are equivalent to the
    // following overload set:
    //
    // /*unspecified*/ send(const OBJECT&);
    //
    // /*unspecified*/ recv(OBJECT&);
    // OBJECT          recv();

    // ACCESSORS
    bool closed() const;

    operator /*unspecified bool*/() const { return !closed(); }
};

template <>
class Chan<void> {
    // Default specialization of 'Chan' for the specific case of a Unix file.

  public:
    // CREATORS
    explicit Chan(int);                   // file descriptor
    explicit Chan(const std::string&);    // file path to open
    Chan(const std::string&, int flags);

    // MANIPULATORS
    void close();

    const FileSend send;
    const FileRecv recv;

    // The two above callable 'const' data member are equivalent to the
    // following overload set:
    //
    // /*unspecified*/ send(const std::string&);
    // /*unspecified*/ send(const char*, std::size_t);
    //
    // /*unspecified*/ recv(std::string*);
    // /*unspecified*/ recv(char*, std::size_t);
    // std::string     recv(std::size_t);

    // ACCESSORS
    bool closed() const;

    std::size_t count() const;
        // Return the size of the last successful 'recv' or 'send' operation.

    operator /*unspecified bool*/() const { return !closed(); }
};

                            // ==============
                            // Chan make(...)
                            // ==============

template <typename OBJECT>
Chan<OBJECT> make();
Chan<>       make(int);  // file descriptor
Chan<>       make(const std::string&);  // file path to open
Chan<>       make(const std::string&, int flags);

                        // =======================
                        // std::size_t select(...)
                        // =======================

template <typename SEQUENCE_OF_EVENT>
std::size_t select(const SEQUENCE_OF_EVENT&);  // e.g. std::vector<Event>

template <std::size_t N>
std::size_t select(const Event (&events)[N]);

template <typename EVENT0, typename EVENT1>
std::size_t select(EVENT0, EVENT1);

template <typename EVENT0, typename EVENT1, typename EVENT2>
std::size_t select(EVENT0, EVENT1, EVENT2);

template <typename EVENT0, typename EVENT1, typename EVENT2, typename EVENT2>
std::size_t select(EVENT0, EVENT1, EVENT2, EVENT3);

// ...

                        // ====================
                        // TimeoutEvent delayMs
                        // ====================

TimeoutEvent delayMs(double milliseconds);

                        // =====================
                        // TimeoutEvent deadline
                        // =====================

TimeoutEvent deadline(Datetime when);

///Concepts
///--------
// The 'chan' library, instead of having a runtime hierarchy if interfaces,
// will use compile-time protocols possibly backed up by type-erased runtime
// interfaces where convenient.
//
///Event
///- - -
// The objects returned by calls to 'send' and 'recv' satisfy the 'Event'
// concept. An 'Event' is something that you can 'select' from. 'Event'
// instances are reference counted, and when the last copy of an event is
// destroyed, if the 'Event' has not yet been "fulfilled" (successfully
// 'selected' from) or "canceled" (unsuccessfully selected from), then the
// destructor will block the calling thread until the event is fulfilled. This
// way, the following code:
//..
//  StockTick tick;
//  channel.recv(&tick);
//..
// blocks until there's a 'StockTick' and then stores it in 'tick'. The actual
// call to 'recv' returned immediately, but then the temporary return 'Event'
// is destroyed, which then blocks until the receive happens.
//
// Similarly,
//..
//  channel.send(StockTick("IBM", 34.24, 34.28));
//..
// blocks for the same reason.
//
// The value-returning version of 'recv' just waits for the receive to finish
// and then returns the value.
//
// There will probably a type-erased wrapper 'class Event' that allows users
// to list their events in a data structure for 'select', instead of always
// having to pass each event as a function argument, e.g.
//..
//  chan::Event selections[] = {foo.recv, bar.send(thing), chan::delayMs(250)};

//  const std::size_t result = select(selections);
//..
//
///Operation
///- - - - -
// An 'Operation' is one of either 'send' or 'recv'. It's its own concept
// because I want users to be able to type 'select(channel.send)' in addition
// to 'select(channel.send(something))', the idea being that 'channel.send' is
// the *operation* of sending, not a particular send. However, I also want
// 'channel.send' to work like a member function invocation. This means that
// 'channel.send' needs to be a data member whose type has 'operator()'
// defined: the type will also satisfy the 'Operation' concept.
//
// An 'Operation' is also an 'Event'. Namely, it's the 'Event' of being capable
// of performing the 'Operation'. But also, the return value of *invoking* an
// 'Operation' is an 'Event'. Namely, the completion of the 'Operation'.

}  // close package namespace
```

[BDE]: https://github.com/bloomberg/bde