
// A <- B
//
// class ChanOp<Receiver, Sender>
//
// class Placeholder;               is _
//
// ChanOp<Placeholder, Chan<T> >    is .recv(x) and discard x
// ChanOp<T, Chan<T> >              is .recv(x)
// ChanOp<Chan<T>, T>               is .send(x)
// ChanOp<Chan<Chan<T> >, Chan<T> > is .send(x)
// ChanOp<Chan<T>, Chan<Chan<T> > > is .recv(x)

namespace chan {

template <typename Object>
class Chan {};

namespace syntax {

class Placeholder {};

const Placeholder _;

template <typename Object>
class Rhs;

template <typename Object>
RecvEvent<Object> operator<(Placeholder, Rhs<Chan<Object> >);

template <typename Object>
RecvEvent<Object> operator<(Object&, Rhs<Chan<Object> >);

template <typename Object>
RecvEvent<Chan<Object> > operator<(Chan<Object>&, Rhs<Chan<Chan<Object> > >);

template <typename Object>
SendEvent<Object> operator<(Chan<Object>&, Rhs<Object>);

template <typename Object>
SendEvent<Object> operator<(Chan<Chan<Object> >&, Rhs<Chan<Object> >);

template <typename Arg>
Rhs<Arg> operator-(Arg&);

// 'Rhs' means "right-hand side".
template <typename Arg>
Rhs<const Arg> operator-(const Arg&);

template <typename Left, typename Right>
ChanOp<Left, Right> operator<(Left&, Rhs<Right>);

template <typename Left, typename Right>
ChanOp<const Left, Right> operator<(const Left&, Rhs<Right>);

// ChanOp is a make-believe template. Instead, the underlying
// type of the send or receive will be what's returned by
// operator<

for (;;) {
    switch (select(foo <- chan, _ <-done)) {
      case 0:
        std::cout << "Read " << foo << " from chan.\n";
        // ...
        if (select(output <- line, _ <-done) == 0)
            continue;
      default:
        std::cout << "Done!\n";
        return;
    }
}

}  // close syntax namespace
}  // close package namespace

