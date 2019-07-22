#ifndef INCLUDED_CHAN_ERRORS_NOEXCEPT
#define INCLUDED_CHAN_ERRORS_NOEXCEPT

namespace chan {

#if __cplusplus >= 201103
#define CHAN_NOEXCEPT noexcept
#define CHAN_THROWS   noexcept(false)
#else
#define CHAN_NOEXCEPT throw()
class Error;
#define CHAN_THROWS   throw(Error)
#endif

}  // namespace chan

#endif
