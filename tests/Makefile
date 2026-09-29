# Host unit tests for the BlockMalloc library
# Usage: make -C tests [CXX=clang++]
#
# Variants of test_blockmalloc.cpp:
#   cpp11, cpp17      AddressSanitizer and UndefinedBehaviorSanitizer
#   cpp11_O2          -O2, no sanitizers
#   custom_default    BLOCK_MALLOC_DEFAULT set by the build
# Every variant links early_user.cpp before BlockMalloc.cpp: a global constructor that
# uses BMalloc during static initialization.
# Also checked: a bad BLOCK_MALLOC_MIN stops the build, and library.json,
# library.properties and BlockMalloc.h carry the same version.

OUT  = _build
SRC  = ../src
LIB  = $(SRC)/BlockMalloc.cpp $(SRC)/BlockMalloc.h
TST  = test_blockmalloc.cpp early_user.cpp
WARN = -Wall -Wextra -Wpedantic -Wshadow -Werror
SAN  = -fsanitize=address,undefined -fno-sanitize-recover=all
BASE = -g $(WARN) -I$(SRC)

# early_user.cpp comes before BlockMalloc.cpp on purpose (see early_user.cpp)
SRCS = test_blockmalloc.cpp early_user.cpp $(SRC)/BlockMalloc.cpp

BINS = $(OUT)/cpp11 $(OUT)/cpp17 $(OUT)/cpp11_O2 $(OUT)/custom_default

test: version config_check $(BINS)
	@for t in $(BINS); do echo "== $$t"; ./$$t || exit 1; done
	@echo "HOST TESTS PASSED"

$(OUT):
	mkdir -p $(OUT)

$(OUT)/cpp11: $(TST) $(LIB) | $(OUT)
	$(CXX) -std=c++11 -O1 $(BASE) $(SAN) $(SRCS) -o $@

$(OUT)/cpp17: $(TST) $(LIB) | $(OUT)
	$(CXX) -std=c++17 -O1 $(BASE) $(SAN) $(SRCS) -o $@

$(OUT)/cpp11_O2: $(TST) $(LIB) | $(OUT)
	$(CXX) -std=c++11 -O2 $(BASE) $(SRCS) -o $@

$(OUT)/custom_default: $(TST) $(LIB) | $(OUT)
	$(CXX) -std=c++11 -O1 $(BASE) $(SAN) -DBLOCK_MALLOC_DEFAULT=64 -DTEST_EXPECTED_DEFAULT=64 $(SRCS) -o $@

config_check:
	@if echo '#include "BlockMalloc.h"' | $(CXX) -std=c++11 -x c++ -fsyntax-only -I$(SRC) -DBLOCK_MALLOC_MIN=0 - 2>/dev/null; then \
	     echo "FAIL config: BLOCK_MALLOC_MIN=0 was accepted"; exit 1; fi
	@if echo '#include "BlockMalloc.h"' | $(CXX) -std=c++11 -x c++ -fsyntax-only -I$(SRC) -DBLOCK_MALLOC_MIN=64 -DBLOCK_MALLOC_MAX=32 - 2>/dev/null; then \
	     echo "FAIL config: BLOCK_MALLOC_MIN > BLOCK_MALLOC_MAX was accepted"; exit 1; fi
	@echo "== config (bad BLOCK_MALLOC_MIN and BLOCK_MALLOC_MAX stop the build)"

version:
	@j=$$(sed -n 's/.*"version": *"\([^"]*\)".*/\1/p' ../library.json); \
	 p=$$(sed -n 's/^version=//p' ../library.properties | tr -d '\r'); \
	 h=$$(sed -n 's/.*BLOCK_MALLOC_VERSION_STRING *"\([^"]*\)".*/\1/p' $(SRC)/BlockMalloc.h); \
	 if [ -z "$$h" ] || [ "$$j" != "$$h" ] || [ "$$p" != "$$h" ]; then \
	     echo "FAIL version: library.json '$$j', library.properties '$$p', BlockMalloc.h '$$h'"; exit 1; fi; \
	 echo "== version $$h (library.json, library.properties and BlockMalloc.h agree)"

clean:
	rm -rf $(OUT)

.PHONY: test version config_check clean
