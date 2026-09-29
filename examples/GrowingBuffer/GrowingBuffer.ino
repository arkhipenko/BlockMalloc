/*
  BlockMalloc: GrowingBuffer

  A buffer that grows as data is appended. The rounded size is the real capacity of
  the buffer, so a new allocation is needed only when the data crosses a block
  boundary, and every buffer the heap sees is a whole number of blocks.
*/

#include <BlockMalloc.h>

BlockMalloc allocator(64);

uint8_t* buffer = NULL;
size_t   used = 0;
size_t   capacity = 0;

// Appends length bytes. Returns false when memory runs out.
static bool append(const uint8_t* bytes, size_t length) {
  size_t needed = used + length;
  if ( needed > capacity ) {
    size_t newCapacity = allocator.roundedSize(needed);
    uint8_t* bigger = (uint8_t*) allocator.allocate(needed);
    if ( bigger == NULL ) return false;
    if ( buffer != NULL ) memcpy(bigger, buffer, used);
    allocator.deallocateSafely(&buffer);
    buffer = bigger;
    capacity = newCapacity;

    Serial.print(F("  capacity is now "));
    Serial.println((unsigned long) capacity);
  }
  memcpy(buffer + used, bytes, length);
  used = needed;
  return true;
}

void setup() {
  Serial.begin(115200);
  while ( !Serial && millis() < 3000 ) {}

  uint8_t chunk[20];
  for ( uint8_t i = 0; i < sizeof(chunk); i++ ) chunk[i] = i;

  for ( int step = 1; step <= 10; step++ ) {
    if ( !append(chunk, sizeof(chunk)) ) {
      Serial.println(F("Out of memory"));
      break;
    }
    Serial.print(F("step "));
    Serial.print(step);
    Serial.print(F(": "));
    Serial.print((unsigned long) used);
    Serial.println(F(" bytes used"));
  }

  allocator.deallocateSafely(&buffer);
  used = capacity = 0;
}

void loop() {
}
