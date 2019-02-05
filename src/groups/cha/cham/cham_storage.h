#ifndef INCLUDED_CHAM_STORAGE
#define INCLUDED_CHAM_STORAGE

#include <bslma_allocator.h>
#include <bslma_default.h>

#include <bsls_assert.h>

namespace cham {
using namespace BloombergLP;

class Storage {
    // This 'class' manages a fixed size buffer of uninitialized memory. It is
    // analogous to 'std::vector<char>', except that the size of the logical
    // sequence can be set, within limits, without altering the buffer or
    // changing storage. The intended use of 'Storage' is as an output
    // parameter to functions like 'read', where a buffer of known size will be
    // written to, but the number of characters written by the function needs
    // also to be indicated, e.g.
    //..
    //  const int     file = ::open(...);
    //  cham::Storage buf(1024 * 1024 * 2);  // lots of room
    //  switch (chan::select(chan::read(file, &buf), ...)) {
    //    case 0:
    //      std::cout << "Read " << buf.size() << " bytes from file with "
    //                   "descriptor " << file << ": "
    //                << bslstl::StringRef(buf.begin(), buf.end()) << '\n';
    //      break;
    //      ...
    //..
    // In the example above, 'chan::read' called 'setSize' on 'buf' to indicate
    // how many characters were written.

    // DATA
    char             *d_buffer_p;
    char             *d_dataEnd_p;
    char             *d_storageEnd_p;
    bslma::Allocator *d_allocator_p;

    // NOT IMPLEMENTED
    Storage(const Storage&);
    Storage& operator=(const Storage&);

  public:
    // CREATORS
    explicit Storage(bsl::size_t size, bslma::Allocator *alloc = 0)
    : d_buffer_p((alloc = bslma::Default::allocator(alloc))->allocate(size))
    , d_dataEnd_p(d_buffer_p + size)
    , d_storageEnd_p(d_dataEnd_p)
    , d_allocator_p(alloc)
    {}

    ~Storage() {
        BSLS_ASSERT(d_allocator_p);
        d_allocator_p->deallocate(d_buffer_p);
    }

    // MANIPULATORS
    void setSize(bsl::size_t newSize) {
        char *const newEnd = d_buffer_p + newSize;
        BSLS_ASSERT(newEnd <= d_storageEnd_p);
        d_storageEnd_p = newEnd;
    }

    char *begin()      { return d_buffer_p; }
    char *end()        { return dataEnd(); }
    char *dataEnd()    { return d_dataEnd_p; }
    char *storageEnd() { return d_storageEnd_p; }

    // ACCESSORS
    const char *begin() const      { return d_buffer_p; }
    const char *end() const        { return dataEnd(); }
    const char *dataEnd() const    { return d_dataEnd_p; }
    const char *storageEnd() const { return d_storageEnd_p; }

    bsl::size_t size() const     { return d_dataEnd_p - d_buffer_p; }
    bsl::size_t capacity() const { return d_storageEnd_p - d_buffer_p; }
};

}  // close package namespace

#endif
