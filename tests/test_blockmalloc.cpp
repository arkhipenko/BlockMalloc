//
// Host unit tests for the BlockMalloc library
// Build and run: make -C tests
//
// The Makefile builds this file in several variants (C++11 and C++17, with and without
// AddressSanitizer and UndefinedBehaviorSanitizer, and with BLOCK_MALLOC_DEFAULT set by
// the build). Under AddressSanitizer malloc_usable_size() returns the exact size given
// to malloc(), so the tests check the size allocate() requested, and a buffer shorter
// than roundedSize() fails the run when it is filled.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <malloc.h>
#include <type_traits>
#include "BlockMalloc.h"

#ifndef TEST_EXPECTED_DEFAULT
#define TEST_EXPECTED_DEFAULT 128
#endif

#if defined(__SANITIZE_ADDRESS__)
#define TEST_ASAN 1
#elif defined(__has_feature)
#if __has_feature(address_sanitizer)
#define TEST_ASAN 1
#endif
#endif

void*  earlyUserPointer();
size_t earlyUserBlockSize();

static int g_fail = 0, g_pass = 0;

#define CHECK(c) do { if (c) g_pass++; else { g_fail++; printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); } } while (0)
#define CHECK_EQ(a, b) do { unsigned long long _a = (unsigned long long)(a), _b = (unsigned long long)(b); \
    if (_a == _b) g_pass++; \
    else { g_fail++; printf("FAIL %s:%d: %s == %s (%llu vs %llu)\n", __FILE__, __LINE__, #a, #b, _a, _b); } } while (0)

// ---- Compile-time checks ----

// The constructor is explicit: BlockMalloc a = 256; must not compile
static_assert(!std::is_convertible<size_t, BlockMalloc>::value, "constructor must be explicit");
// Trivially destructible, so the global object needs no destructor registration
static_assert(std::is_trivially_destructible<BlockMalloc>::value, "BlockMalloc must be trivially destructible");
// constexpr construction, clamping and rounding
static_assert(BlockMalloc(100).getBlockSize() == 100, "constexpr constructor");
static_assert(BlockMalloc(0).getBlockSize() == BLOCK_MALLOC_MIN, "constexpr clamp low");
static_assert(BlockMalloc(1000000).getBlockSize() == BLOCK_MALLOC_MAX, "constexpr clamp high");
static_assert(BlockMalloc(128).roundedSize(129) == 256, "constexpr roundedSize");

// ---- Helpers ----

// Size malloc() was asked for, as far as the host heap can tell
static void check_heap_size(void* p, size_t expected, int line) {
    size_t usable = malloc_usable_size(p);
#ifdef TEST_ASAN
    if (usable == expected) g_pass++;
    else { g_fail++; printf("FAIL line %d: malloc_usable_size %zu, expected %zu\n", line, usable, expected); }
#else
    if (usable >= expected) g_pass++;
    else { g_fail++; printf("FAIL line %d: malloc_usable_size %zu, expected at least %zu\n", line, usable, expected); }
#endif
}

// ---- Tests ----

static void test_version() {
    int major = -1, minor = -1, patch = -1;
    CHECK_EQ(sscanf(BLOCK_MALLOC_VERSION_STRING, "%d.%d.%d", &major, &minor, &patch), 3);
    CHECK_EQ(BLOCK_MALLOC_VERSION, major * 10000 + minor * 100 + patch);
}

static void test_constants() {
    CHECK_EQ(BLOCK_MALLOC_DEFAULT, TEST_EXPECTED_DEFAULT);
    CHECK_EQ(BLOCK_MALLOC_MIN, 16);
    CHECK_EQ(BLOCK_MALLOC_MAX, 4096);
}

static void test_construction_and_clamping() {
    CHECK_EQ(BlockMalloc().getBlockSize(), TEST_EXPECTED_DEFAULT);
    CHECK_EQ(BMalloc.getBlockSize(), TEST_EXPECTED_DEFAULT);
    CHECK_EQ(BlockMalloc(0).getBlockSize(), 16);
    CHECK_EQ(BlockMalloc(15).getBlockSize(), 16);
    CHECK_EQ(BlockMalloc(16).getBlockSize(), 16);
    CHECK_EQ(BlockMalloc(100).getBlockSize(), 100);     // not a power of two: kept
    CHECK_EQ(BlockMalloc(4096).getBlockSize(), 4096);
    CHECK_EQ(BlockMalloc(4097).getBlockSize(), 4096);
    CHECK_EQ(BlockMalloc(SIZE_MAX).getBlockSize(), 4096);

    BlockMalloc a;
    CHECK_EQ(a.setBlockSize(0), 16);
    CHECK_EQ(a.getBlockSize(), 16);
    CHECK_EQ(a.setBlockSize(5000), 4096);
    CHECK_EQ(a.getBlockSize(), 4096);
    CHECK_EQ(a.setBlockSize(256), 256);
    CHECK_EQ(a.getBlockSize(), 256);
}

// SPEC-0009 section 6 worked example
static void test_rounding_table() {
    BlockMalloc a(128);
    CHECK_EQ(a.roundedSize(0), 0);
    CHECK_EQ(a.roundedSize(1), 128);
    CHECK_EQ(a.roundedSize(127), 128);
    CHECK_EQ(a.roundedSize(128), 128);
    CHECK_EQ(a.roundedSize(129), 256);
    CHECK_EQ(a.roundedSize(300), 384);

    BlockMalloc b(100);
    CHECK_EQ(b.roundedSize(1), 100);
    CHECK_EQ(b.roundedSize(100), 100);
    CHECK_EQ(b.roundedSize(101), 200);
}

static void test_allocate_sizes() {
    BlockMalloc a(128);
    CHECK(a.allocate(0) == NULL);

    const size_t requests[] = { 1, 127, 128, 129, 300, 1000 };
    for (size_t i = 0; i < sizeof(requests) / sizeof(requests[0]); i++) {
        size_t n = requests[i];
        void* p = a.allocate(n);
        CHECK(p != NULL);
        if ( p == NULL ) continue;
        check_heap_size(p, a.roundedSize(n), __LINE__);
        memset(p, 0xA5, a.roundedSize(n));             // the whole rounded size is usable
        a.deallocate(p);
    }
}

// D1: rounding must never wrap to a small size
static void test_overflow_guard() {
    const size_t sizes[] = { 16, 100, 128, 4095, 4096 };
    for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
        BlockMalloc a(sizes[i]);
        size_t b = a.getBlockSize();
        size_t largest = SIZE_MAX - (b - 1);            // largest request that is rounded

        CHECK_EQ(a.roundedSize(SIZE_MAX), 0);
        CHECK_EQ(a.roundedSize(largest + 1), 0);
        size_t r = a.roundedSize(largest);
        CHECK(r >= largest);
        CHECK_EQ(r % b, 0);

        CHECK(a.allocate(SIZE_MAX) == NULL);
        CHECK(a.allocate(largest + 1) == NULL);
    }

    // A typical trigger: unsigned underflow of a length
    size_t len = 2, hdr = 4;
    CHECK(BMalloc.allocate(len - hdr) == NULL);
}

static void test_interchangeable_with_malloc() {
    BlockMalloc a(64), b(256);

    void* p = a.allocate(10);
    CHECK(p != NULL);
    free(p);                                          // plain free() of allocate() memory

    p = malloc(10);
    CHECK(p != NULL);
    a.deallocate(p);                                  // deallocate() of malloc() memory

    p = a.allocate(10);
    b.deallocate(p);                                  // another object releases it

    a.deallocate(NULL);                               // ignored
    CHECK(true);
}

struct Record { uint32_t id; char name[20]; };

static void test_deallocate_safely() {
    void* v = BMalloc.allocate(10);
    CHECK(v != NULL);
    BMalloc.deallocateSafely(&v);
    CHECK(v == NULL);
    BMalloc.deallocateSafely(&v);                     // already NULL: no effect
    CHECK(v == NULL);

    // D3: typed pointers without a cast
    uint8_t* u = (uint8_t*) BMalloc.allocate(10);
    CHECK(u != NULL);
    BMalloc.deallocateSafely(&u);
    CHECK(u == NULL);

    char* c = (char*) BMalloc.allocate(10);
    BMalloc.deallocateSafely(&c);
    CHECK(c == NULL);

    const char* cc = (const char*) BMalloc.allocate(10);
    BMalloc.deallocateSafely(&cc);
    CHECK(cc == NULL);

    Record* r = (Record*) BMalloc.allocate(sizeof(Record));
    BMalloc.deallocateSafely(&r);
    CHECK(r == NULL);

    // D4: a NULL address is ignored, typed or not
    BMalloc.deallocateSafely(NULL);
    BMalloc.deallocateSafely((void**) NULL);
    uint8_t** none = NULL;
    BMalloc.deallocateSafely(none);
    CHECK(true);
}

// D2: BMalloc must be usable from a global constructor in another file
static void test_static_initialization() {
    CHECK(earlyUserPointer() != NULL);
    CHECK_EQ(earlyUserBlockSize(), TEST_EXPECTED_DEFAULT);
    BMalloc.deallocate(earlyUserPointer());
}

int main() {
    test_version();
    test_constants();
    test_construction_and_clamping();
    test_rounding_table();
    test_allocate_sizes();
    test_overflow_guard();
    test_interchangeable_with_malloc();
    test_deallocate_safely();
    test_static_initialization();

    printf("%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
