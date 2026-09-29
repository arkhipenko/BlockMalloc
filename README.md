# BlockMalloc

A `malloc()` wrapper for microcontrollers that rounds every request up to a multiple of a block size.

## Description

`allocate(n)` asks the heap for `n` rounded up to a whole number of blocks (default 128 bytes). Freed heap chunks then come in fewer distinct sizes, so a later request of similar size is more likely to fit into a hole left by an earlier one. This can help a program that repeatedly frees and allocates buffers of similar but varying size, such as message or text buffers.

The heap itself is the platform heap. BlockMalloc does not prevent fragmentation, holds no memory pool and tracks no allocations. The cost is up to one block minus one byte of unused memory per allocation: with 128-byte blocks, `allocate(1)` takes 128 bytes of heap.

Memory from BlockMalloc is ordinary `malloc()` memory. It may be released with `free()`, and `deallocate()` accepts any pointer from `malloc()`.

## Features

- Size rounding to a block size of 16 to 4096 bytes (default 128), per allocator object.
- Overflow-safe rounding: a request too large to round returns NULL.
- `deallocateSafely()` releases memory and sets the caller's pointer to NULL, for any pointer type.
- `roundedSize()` reports the size a request occupies. The whole rounded size is usable.
- A shared global object `BMalloc` that is initialized at compile time, so it may be used from other global constructors.
- No dependency on the Arduino API. Plain C++11.

## Installation

- Arduino IDE: download the repository as a ZIP file and use Sketch > Include Library > Add .ZIP Library, or clone it into your `libraries` folder.
- PlatformIO: add it to `platformio.ini`:

```ini
lib_deps = https://github.com/arkhipenko/BlockMalloc.git#v1.1.0
```

## Usage

```cpp
#include <BlockMalloc.h>

// A private allocator with 64-byte blocks
BlockMalloc buffers(64);

void setup() {
  Serial.begin(115200);

  // The shared global object, 128-byte blocks
  char* text = (char*) BMalloc.allocate(40);      // 128 bytes requested from the heap
  if (text) {
    strcpy(text, "hello");
    Serial.println(text);
    BMalloc.deallocateSafely(&text);             // text is now NULL
  }

  uint8_t* data = (uint8_t*) buffers.allocate(100);   // 128 bytes: 2 blocks of 64
  if (data) {
    memset(data, 0, buffers.roundedSize(100));  // all 128 bytes are usable
    buffers.deallocateSafely(&data);
  }
}

void loop() {
}
```

More in `examples/`: Basic, CustomBlockSize and GrowingBuffer.

## API Reference

### Constructor

- `explicit BlockMalloc(size_t block_size = BLOCK_MALLOC_DEFAULT)` - creates an allocator. The block size is clamped to `BLOCK_MALLOC_MIN`..`BLOCK_MALLOC_MAX`.

### Methods

- `void* allocate(size_t requestedSize)` - returns `roundedSize(requestedSize)` bytes from `malloc()`. Returns NULL when `requestedSize` is 0, when rounding would overflow, or when `malloc()` fails. The memory is not zeroed.
- `void deallocate(void* pointer)` - calls `free()`. NULL is ignored.
- `void deallocateSafely(T** pointer)` - calls `free()` on `*pointer` and sets `*pointer` to NULL. Works for `void*`, `char*`, `uint8_t*` and any other pointer type. A NULL address is ignored.
- `size_t setBlockSize(size_t block_size)` - sets the block size for later allocations and returns the value actually set (clamped).
- `size_t getBlockSize()` - returns the block size.
- `size_t roundedSize(size_t requestedSize)` - returns the size `allocate()` requests from the heap: `requestedSize` rounded up to a multiple of the block size, or 0 when `requestedSize` is 0 or too large to round.

### Constants

- `BLOCK_MALLOC_DEFAULT` (128) - block size of `BMalloc` and of the default constructor.
- `BLOCK_MALLOC_MIN` (16) - smallest block size.
- `BLOCK_MALLOC_MAX` (4096) - largest block size.
- `BLOCK_MALLOC_VERSION` (10100) and `BLOCK_MALLOC_VERSION_STRING` ("1.1.0") - library version.

The three sizes may be set by the build, for example `build_flags = -D BLOCK_MALLOC_DEFAULT=64` in PlatformIO.

### Global Instance

- `BMalloc` - allocator with block size `BLOCK_MALLOC_DEFAULT`, shared by all code in the firmware.

## Notes

- `BMalloc` is shared. A library that calls `BMalloc.setBlockSize()` changes the rounding for every other user. A library should create its own `BlockMalloc` object.
- BlockMalloc adds no locking. `allocate()` and `deallocate()` are as safe as the platform `malloc()` and `free()`. Do not call `setBlockSize()` on an object while another task or an interrupt may call `allocate()` on the same object.
- Rounding uses a division. AVR has no hardware divider, so each `allocate()` costs a software division there.

## Version history

- 1.1.0 (2026-09-29): overflow-safe rounding (a huge request could return a small buffer); `BMalloc` initialized at compile time (a global constructor that used it could divide by zero); `deallocateSafely()` for typed pointers and NULL-safe; `getBlockSize()`, `roundedSize()`, version macros; the size constants may be set by the build; no Arduino API dependency; the constructor is `explicit`; examples, host tests and CI; corrected documentation.
- 1.0.0 (2025-11-02): first release.

## Author

Anatoli Arkhipenko <arkhipenko@hotmail.com>

## License

BSD 3-Clause. See LICENSE.txt.

## Repository

https://github.com/arkhipenko/BlockMalloc
