TODO
====
```c++
class File {
    File();

    explicit File(int);

    bool closeOnDestroy;

    bool isOpen() const;
    operator void*() const;

    OpenResult open(const char*);
    OpenResult open(const std::string&);

    void close();

    ReadIntoBufferEvent read(char*, int);
    ReadIntoStringEvent read(std::string&);

    WriteFromBufferEvent write(const char*, int);
    WriteFromStringEvent write(const std::string&);

    int gcount();
    int pcount();
};
```