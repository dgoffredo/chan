#ifndef INCLUDED_CHAN_ERRORS_NOEXCEPT
#define INCLUDED_CHAN_ERRORS_NOEXCEPT

#if __cpluscplus >= 201103
    #define CHAN_NOEXCEPT noexcept
#else
    #define CHAN_NOEXCEPT throw()
#endif

#endif

