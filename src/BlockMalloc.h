#pragma once

#include <stdint.h>
#include <stddef.h>

/** @brief Library version as a number: major * 10000 + minor * 100 + patch */
#define BLOCK_MALLOC_VERSION            10100
/** @brief Library version as a string */
#define BLOCK_MALLOC_VERSION_STRING     "1.1.0"

// The three sizes below may be set by the build, e.g. -D BLOCK_MALLOC_DEFAULT=64

#ifndef BLOCK_MALLOC_DEFAULT
/** @brief Block size of BMalloc and of the default constructor, bytes */
#define BLOCK_MALLOC_DEFAULT    128
#endif

#ifndef BLOCK_MALLOC_MIN
/** @brief Smallest accepted block size, bytes */
#define BLOCK_MALLOC_MIN        16
#endif

#ifndef BLOCK_MALLOC_MAX
/** @brief Largest accepted block size, bytes */
#define BLOCK_MALLOC_MAX        4096
#endif

#if BLOCK_MALLOC_MIN < 1
#error "BLOCK_MALLOC_MIN must be at least 1"
#endif
#if BLOCK_MALLOC_MIN > BLOCK_MALLOC_MAX
#error "BLOCK_MALLOC_MIN must not exceed BLOCK_MALLOC_MAX"
#endif

/**
 * @brief malloc() wrapper that rounds every request up to a multiple of a block size
 *
 * Freed heap chunks then come in fewer distinct sizes, which can help a heap that
 * repeatedly frees and allocates buffers of similar size. The heap itself is the
 * platform heap: this class does not prevent fragmentation, holds no pool and tracks
 * no allocations. Memory it returns may be released with free(), and deallocate()
 * accepts any pointer from malloc().
 */
class BlockMalloc {
    public:
        /**
         * @brief Constructor for BlockMalloc
         * @param block_size Block size in bytes (default: BLOCK_MALLOC_DEFAULT)
         * @note The block size is clamped to BLOCK_MALLOC_MIN..BLOCK_MALLOC_MAX.
         *       The constructor is constexpr, so a global object is initialized at
         *       compile time and is usable from other global constructors.
         */
        explicit constexpr BlockMalloc(size_t block_size = BLOCK_MALLOC_DEFAULT)
            : m_blockSize(clampBlockSize(block_size)) {}

        /**
         * @brief Allocate memory of requested size
         * @param requestedSize Number of bytes to allocate
         * @return Pointer to roundedSize(requestedSize) bytes, or NULL when requestedSize
         *         is 0, when rounding would overflow, or when malloc() fails
         * @note The memory is not zeroed. The caller may use all roundedSize() bytes.
         */
        void* allocate(size_t requestedSize);

        /**
         * @brief Release memory
         * @param pointer Pointer from allocate() of any BlockMalloc object, or from
         *        malloc(). NULL is ignored.
         */
        void  deallocate(void* pointer);

        /**
         * @brief Release memory and set the caller's pointer to NULL
         * @param pointer Address of the caller's pointer. NULL is ignored.
         */
        void  deallocateSafely(void** pointer);

        /**
         * @brief Release memory and set the caller's typed pointer to NULL
         * @param pointer Address of the caller's pointer, e.g. &buffer for a uint8_t*
         *        buffer. NULL is ignored.
         */
        template <typename T>
        void  deallocateSafely(T** pointer) {
            if ( pointer == NULL ) return;
            deallocate((void*) *pointer);
            *pointer = NULL;
        }

        /**
         * @brief Set the block size for future allocations
         * @param block_size New block size (clamped to BLOCK_MALLOC_MIN..BLOCK_MALLOC_MAX)
         * @return Block size actually set
         * @note Does not affect memory already allocated. On the shared BMalloc object
         *       this changes the rounding for every user in the firmware.
         */
        size_t setBlockSize(size_t block_size);

        /**
         * @brief Current block size
         * @return Block size in bytes
         */
        constexpr size_t getBlockSize() const { return m_blockSize; }

        /**
         * @brief Size that allocate() requests from the heap for a given request
         * @param requestedSize Number of bytes requested
         * @return requestedSize rounded up to a multiple of the block size, or 0 when
         *         requestedSize is 0 or rounding would overflow size_t
         */
        constexpr size_t roundedSize(size_t requestedSize) const {
            return ( requestedSize == 0 || requestedSize > (size_t) -1 - (m_blockSize - 1) )
                   ? 0
                   : ( (requestedSize + m_blockSize - 1) / m_blockSize ) * m_blockSize;
        }

    private:
        static constexpr size_t clampBlockSize(size_t block_size) {
            return block_size < (size_t) BLOCK_MALLOC_MIN ? (size_t) BLOCK_MALLOC_MIN
                 : block_size > (size_t) BLOCK_MALLOC_MAX ? (size_t) BLOCK_MALLOC_MAX
                 : block_size;
        }

        size_t  m_blockSize;  ///< Current block size for allocations
};

/** @brief Global instance of BlockMalloc with block size BLOCK_MALLOC_DEFAULT, shared by all users */
extern BlockMalloc BMalloc;
