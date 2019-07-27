#include <chan/debug/trace.h>
#include <chan/fileevents/ignoresigpipe.h>

#include <signal.h>

namespace chan {

void* ignoreSigpipe() {
    // Based on Louic Domaigne's answer to:
    // https://www.unix.com/programming/
    //               132681-reliable-management-signal-sigpipe-sigterm.html
    // accessed July 26, 2019.
    struct sigaction signalAction;
    signalAction.sa_handler = SIG_IGN;
    sigemptyset(&signalAction.sa_mask);
    signalAction.sa_flags = 0;
    sigaction(SIGPIPE, &signalAction, 0);

    CHAN_TRACE("chan::IgnoreSigpipe set SIGPIPE to SIG_IGN.");
    return 0;
}

void* sigpipeIgnorer = ignoreSigpipe();

}  // namespace chan
