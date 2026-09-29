/*
  BlockMalloc: Basic

  Uses the shared global instance BMalloc (block size 128 bytes) to allocate a text
  buffer, and deallocateSafely() to release it and clear the pointer.
*/

#include <BlockMalloc.h>

void setup() {
  Serial.begin(115200);
  while ( !Serial && millis() < 3000 ) {}

  Serial.print(F("BlockMalloc "));
  Serial.println(F(BLOCK_MALLOC_VERSION_STRING));

  const size_t length = 40;
  char* text = (char*) BMalloc.allocate(length);
  if ( text == NULL ) {
    Serial.println(F("Allocation failed"));
    return;
  }

  // The heap was asked for a whole number of blocks
  Serial.print(F("Requested "));
  Serial.print((unsigned long) length);
  Serial.print(F(" bytes, the heap holds "));
  Serial.print((unsigned long) BMalloc.roundedSize(length));
  Serial.println(F(" bytes"));

  snprintf(text, length, "uptime %lu ms", (unsigned long) millis());
  Serial.println(text);

  // Releases the buffer and sets text to NULL, so it cannot be used again by mistake
  BMalloc.deallocateSafely(&text);
  Serial.println(text == NULL ? F("Pointer cleared") : F("Pointer NOT cleared"));
}

void loop() {
}
