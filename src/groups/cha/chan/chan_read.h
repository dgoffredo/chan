
ReadEvent read(int          file,
               char        *buffer,
               bsl::size_t  size,
               bsl::size_t *readSize = 0);

ReadEvent read(int file, bsl::string       *storage);
ReadEvent read(int file, bsl::vector<char> *storage);

// The previous two can be consolidated:
template <typename RESIZABLE>
ReadEvent read(int file, RESIZABLE *storage);

// This is best, but non-standard:
ReadEvent read(int file, cham::Storage *storage);
