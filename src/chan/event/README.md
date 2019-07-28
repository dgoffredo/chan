The Event Package
=================

The _Event_ Concept
-------------------
`chan::select` returns the index of the first and only of its arguments to be
"fulfilled," or returns a negative error code.

Its arguments are objects that satisfy the _event_ concept.  An _event_
implements the following member functions:

### `void touch() CHAN_NOEXCEPT`
`chan::select` invokes this member function exactly once on each of its
arguments, before doing anything else.  It notifies the event that the event
is now part of a call to `chan::select`.  Some event objects will block in
their destructor unless `touch` has been called on them.

### `IoEvent file(const EventContext&)`
After calling `touch`, `chan::select` calls `file` once on each event.  This
is how the event communicates the initial `IoEvent` on which `chan::select`
will wait on the event's behalf.  See the "`IoEvent` Objects" section, below,
for a description of `IoEvent`.

The sole argument to `file`, `EventContext`, allows the event to manage
relationships among different concurrent calls to `chan::select`.
`EventContext` is necessary in order to negotiate `Chan` sends and receives.
See the "`EventContext` Objects" section, below, for a description of
`EventContext`.

The event indicates an error by throwing an exception.

### `IoEvent fulfill(IoEvent)`
When the `IoEvent` associated with an event becomes available, `chan::select`
will call `fulfill`, passing the relevant `IoEvent` as the sole argument.  The
event may then perform whatever action it associates with the `IoEvent`, and
return an `IoEvent` in return.  The returned `IoEvent` falls into one of the
following two cases:

1. If the `IoEvent`'s `fulfill` flag is set, then the event will be considered
   fulfilled, `cancel` will be called on all other events in the `chan::select`
   call, and `chan::select` will return the argument index of the fulfilled
   event.
2. If the `IoEvent`'s `fulfill` flag is not set, then the `IoEvent` will take
   the place of that previously returned by a call to the event's `file`
   method.  `fulfill` will be called again with this new `IoEvent` if it
   becomes available.

The event indicates an error by throwing an exception.

### `void cancel(IoEvent)`
When an event is fulfilled, `chan::select` will call `cancel` on all of the
other events.  Additionally, if an exception is thrown within or below
`chan::select`, `cancel` will be called on every event that has previously had
`file` called on it.

The argument to `cancel` is the `IoEvent` returned most recently by the event,
either from `file` or from `fulfill`.

`IoEvent` Objects
-----------------
`IoEvent` has the following members:

    bool read
    bool write
    bool timeout
    bool fulfilled
    bool hangup
    bool error
    bool invalid

    int       file
    TimePoint expiration

`IoEvent` serves two roles.

When an `IoEvent` returned from a call to the event methods
`file` or `fulfill`, it conveys to `chan::select` on what condition it should
wait.  If `fulfilled` is set, then `chan::select` will consider the event
fulfilled and return to the caller (after calling `cancel` on all other
events).  If `timeout` is set, then the `IoEvent` indicates that `chan::select`
will increase its timeout to at most until `expiration`.  If either or both of
`read` or `write` are set, then the `IoEvent` indicates that `chan::select`
will add the `file` descriptor to its polling set, monitoring `file` for the
relevant capability (readability, writability, or both).  The following flags
are ignored: `hangup`, `error`, and `invalid`.

When an `IoEvent` is passed as the argument to a call to the event methods
`fulfill` or `cancel`, it conveys the last known status of the `IoEvent`
previously returned by the event's `file` or `fulfill` methods.  It will have
the same value as the previously returned object, possibly with the following
exceptions:

- If either or both of `read` or `write` is set, and if there was an error on
  the `file`, then `error` will be set.  One possible error is that `file` is
  a pipe or FIFO open for writing, and the last remaining reader closed the
  other end.
- If `read` is set and `file` is a pipe or FIFO whose writer has closed the
  other end, `hangup` will be set.
- If either or both of `read` or `write` is set, and if `file` is not a valid
  file descriptor (e.g. is closed), then `invalid` will be set.

`EventContext` Objects
----------------------
See the comments in `eventcontext.h`.  `EventContext` is a feature of
`chan::select` that was necessary in order to implements channels
(`class Chan`).  Proper use of `EventContext` involves a mutex locking protocol
and assumptions about the state of those mutexes throughout the operation of
`chan::select`.  Events that involve files only (see `chan/fileevents`) ignore
`EventContext` completely.