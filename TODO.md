TODO
====
The Deadlock
------------
What to do? Here are some notes:

- randomize records
- file() -> lock channel. ready? done. else, sitter.
    - if not a channel, try the IoEvent once (zero timeout)
- nothing channels ready? unlock all channels.
    - is this a new step in the protocol: prepare()?
- wait to be woken up
- woken -> fulfill()
    - if fulfilled:
        - lock all channels (except the one)
        - cancel all channels (except the one)
        - unlock all channels (except the one)
    - if not, wait to be woken up
```C++
struct IoEvent {
    bool ___ : 1;
    bool ___ : 1;
    // ...

    Mutex     *mutex;
    int        file;
    TimePoint  expiration;
};
```

Maybe the point is that the "to me" pipe has to be the same across events,
and when you `read` a message, you have to be able to determine who sent it.

```C++
writeMessage(me.pipes->toVisitor, ChanProtocolMessage::READY);
::poll([me.pipes->fromVisitor, ...])'
// GAH, THIS MODEL DOESN'T WORK AT ALL!
```
