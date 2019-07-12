struct IoEvents {
    bool read : 1;
    bool write : 1;
    bool hangup : 1;
    bool timeout : 1;
    bool fulfill : 1;

    union {
        int file;
        int milliseconds;
    };

    IoEvents()
    : read(), write(), hangup(), timeout(), fulfill()
    {}
};

int main() {
    IoEvents events;
    events.read = true;
    events.file = 0;
    
    return events.write;
}
