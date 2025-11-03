#include <Arduino.h>
#include "BlockMalloc.h"

BlockMalloc::BlockMalloc(size_t block_size) {
    setBlockSize(block_size);
}

BlockMalloc::~BlockMalloc(){}

size_t BlockMalloc::setBlockSize(size_t block_size) {
    m_blockSize = constrain(block_size, BLOCK_MALLOC_MIN, BLOCK_MALLOC_MAX);
    return m_blockSize;
}

void* BlockMalloc::allocate(size_t requestedSize) {

    if ( requestedSize == 0 ) return NULL;
    size_t noOfBlocks = requestedSize / m_blockSize;
    if ( requestedSize % m_blockSize > 0 ) noOfBlocks++;
    void* p = malloc(noOfBlocks * m_blockSize);

    return p;
}

void  BlockMalloc::deallocate(void* pointer) {
    if (pointer != NULL ) free(pointer);
}

void  BlockMalloc::deallocateSafely(void** pointer) {
    if ( *pointer != NULL ) {
        free(*pointer);
        *pointer = NULL;
    }
}

BlockMalloc BMalloc(BLOCK_MALLOC_DEFAULT);