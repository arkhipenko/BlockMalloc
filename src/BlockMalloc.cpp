#include <stdlib.h>
#include "BlockMalloc.h"

size_t BlockMalloc::setBlockSize(size_t block_size) {
    m_blockSize = clampBlockSize(block_size);
    return m_blockSize;
}

void* BlockMalloc::allocate(size_t requestedSize) {
    size_t size = roundedSize(requestedSize);
    if ( size == 0 ) return NULL;
    return malloc(size);
}

void  BlockMalloc::deallocate(void* pointer) {
    free(pointer);
}

void  BlockMalloc::deallocateSafely(void** pointer) {
    if ( pointer == NULL ) return;
    free(*pointer);
    *pointer = NULL;
}

// Constant-initialized (constexpr constructor): no global constructor runs for it
BlockMalloc BMalloc(BLOCK_MALLOC_DEFAULT);
