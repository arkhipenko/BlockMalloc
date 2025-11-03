# BlockMalloc

MCU-friendly memory allocation module for Arduino projects.

## Description

BlockMalloc provides memory allocation in blocks of bytes to prevent fragmentation, making it ideal for microcontroller environments where memory management is critical.

## Features

- Block-based memory allocation to prevent fragmentation
- Configurable block sizes (16-4096 bytes, default 128)
- Safe deallocation methods
- Lightweight and MCU-friendly design

## Installation

1. Download the library
2. Place it in your Arduino libraries folder
3. Include it in your sketch: `#include <BlockMalloc.h>`

## Usage

```cpp
#include <BlockMalloc.h>

void setup() {
  Serial.begin(9600);
  
  // Use the global instance
  void* ptr = BMalloc.allocate(64);
  if (ptr) {
    Serial.println("Memory allocated successfully");
    BMalloc.deallocate(ptr);
  }
  
  // Or create your own instance with custom block size
  BlockMalloc customAlloc(256);
  void* ptr2 = customAlloc.allocate(100);
  if (ptr2) {
    customAlloc.deallocateSafely(&ptr2); // Safer option
  }
}

void loop() {
  // Your code here
}
```

## API Reference

### Constructor
- `BlockMalloc(size_t block_size = BLOCK_MALLOC_DEFAULT)` - Create allocator with specified block size

### Methods
- `void* allocate(size_t requestedSize)` - Allocate memory of requested size
- `void deallocate(void* pointer)` - Deallocate memory pointer
- `void deallocateSafely(void** pointer)` - Safely deallocate and nullify pointer
- `size_t setBlockSize(size_t block_size)` - Set new block size

### Constants
- `BLOCK_MALLOC_DEFAULT` (128) - Default block size
- `BLOCK_MALLOC_MIN` (16) - Minimum block size  
- `BLOCK_MALLOC_MAX` (4096) - Maximum block size

### Global Instance
- `BMalloc` - Pre-instantiated global allocator with default settings

## Author

Anatoli Arkhipenko <arkhipenko@hotmail.com>

## License

This library is open source. Please check the repository for license details.

## Repository

https://github.com/arkhipenko/BlockMalloc