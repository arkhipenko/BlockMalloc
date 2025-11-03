#pragma once

#include <stdint.h>
#include <stddef.h>

/** @brief Default block size for memory allocation */
#define BLOCK_MALLOC_DEFAULT    128
/** @brief Minimum allowed block size */
#define BLOCK_MALLOC_MIN        16
/** @brief Maximum allowed block size */
#define BLOCK_MALLOC_MAX        4096

/**
 * @brief Block-based memory allocator to prevent fragmentation
 * 
 * This class provides a memory allocation system that allocates memory in 
 * fixed-size blocks to prevent fragmentation, making it suitable for 
 * microcontroller environments with limited memory.
 */
class BlockMalloc {
    public:
        /**
         * @brief Constructor for BlockMalloc
         * @param block_size Size of memory blocks to allocate (default: BLOCK_MALLOC_DEFAULT)
         * @note Block size is constrained between BLOCK_MALLOC_MIN and BLOCK_MALLOC_MAX
         */
        BlockMalloc(size_t block_size = BLOCK_MALLOC_DEFAULT);
        
        /**
         * @brief Destructor for BlockMalloc
         * @note Cleans up any allocated memory blocks
         */
        ~BlockMalloc();

        /**
         * @brief Allocate memory of requested size
         * @param requestedSize Number of bytes to allocate
         * @return Pointer to allocated memory, or nullptr if allocation fails
         * @note Memory is allocated in blocks, so actual allocated size may be larger than requested
         */
        void* allocate(size_t requestedSize);
        
        /**
         * @brief Deallocate previously allocated memory
         * @param pointer Pointer to memory block to deallocate
         * @warning Pointer must have been allocated by this allocator instance
         */
        void  deallocate(void* pointer);
        
        /**
         * @brief Safely deallocate memory and nullify pointer
         * @param pointer Pointer to pointer to memory block to deallocate
         * @note After deallocation, the pointer is set to nullptr for safety
         */
        void  deallocateSafely(void** pointer);
        
        /**
         * @brief Set new block size for future allocations
         * @param block_size New block size (constrained between BLOCK_MALLOC_MIN and BLOCK_MALLOC_MAX)
         * @return Actual block size set (may differ from requested if out of bounds)
         * @note Does not affect previously allocated blocks
         */
        size_t setBlockSize(size_t block_size);

    private:
        size_t  m_blockSize;  ///< Current block size for allocations
};

/** @brief Global instance of BlockMalloc with default settings */
extern BlockMalloc BMalloc;
