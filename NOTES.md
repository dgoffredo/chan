Channel Protocol Designs
========================

Use the Filesystem
------------------

    /tmp/<guid>/channel_jkskjfkdd/
                                  receivers
                                  senders/
                                          djfksljfd
                                          odsfjksal

    // clean up with atexit?
    // This is a terrible idea but I can see it working.

    // Is there a "is open for writing".
    // God this design is terrible.

`list` of `pipe()` with `Mutex`
-------------------------------

    class ChanImpl {
        list<Pipes>  d_senders;
        list<Pipes>  d_receivers;
        bslmt::Mutex d_mutex;

        enum { e_SENDER, e_RECEIVER } d_waiter;

        // TODO
    };
