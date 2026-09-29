//
// A global object whose constructor uses BMalloc during static initialization.
// The Makefile links this file before BlockMalloc.cpp. In 1.0.0 BMalloc was initialized
// at run time, so this constructor saw a block size of 0 and divided by it (SIGFPE).
//

#include "BlockMalloc.h"

struct EarlyUser {
    void*  pointer;
    size_t blockSize;
    EarlyUser() : pointer(BMalloc.allocate(64)), blockSize(BMalloc.getBlockSize()) {}
};

EarlyUser g_earlyUser;

void*  earlyUserPointer()   { return g_earlyUser.pointer; }
size_t earlyUserBlockSize() { return g_earlyUser.blockSize; }
