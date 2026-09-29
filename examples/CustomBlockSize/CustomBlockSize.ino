/*
  BlockMalloc: CustomBlockSize

  A library or a module should use its own BlockMalloc object rather than change the
  block size of the shared BMalloc. This sketch creates a private object, shows how the
  block size is clamped, and prints how requests are rounded.
*/

#include <BlockMalloc.h>

// A private allocator with 32-byte blocks
BlockMalloc smallBlocks(32);

static void printRounding(BlockMalloc& allocator, size_t request) {
  Serial.print(F("  "));
  Serial.print((unsigned long) request);
  Serial.print(F(" -> "));
  Serial.println((unsigned long) allocator.roundedSize(request));
}

void setup() {
  Serial.begin(115200);
  while ( !Serial && millis() < 3000 ) {}

  Serial.print(F("Block size: "));
  Serial.println((unsigned long) smallBlocks.getBlockSize());

  Serial.println(F("Rounding:"));
  printRounding(smallBlocks, 1);
  printRounding(smallBlocks, 32);
  printRounding(smallBlocks, 33);
  printRounding(smallBlocks, 100);

  // Out-of-range sizes are clamped to BLOCK_MALLOC_MIN..BLOCK_MALLOC_MAX
  Serial.print(F("setBlockSize(1) sets "));
  Serial.println((unsigned long) smallBlocks.setBlockSize(1));
  Serial.print(F("setBlockSize(10000) sets "));
  Serial.println((unsigned long) smallBlocks.setBlockSize(10000));
  smallBlocks.setBlockSize(32);

  // Memory from any BlockMalloc object may be released by any other, or by free()
  uint8_t* data = (uint8_t*) smallBlocks.allocate(50);
  if ( data != NULL ) {
    memset(data, 0, smallBlocks.roundedSize(50));   // all 64 bytes are usable
    BMalloc.deallocateSafely(&data);
  }
  Serial.println(data == NULL ? F("Released") : F("Not released"));
}

void loop() {
}
