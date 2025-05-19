# Use clang (or change to gcc)
CC       := clang

# SIMD options (set with make SIMD=[option])
# - none: No SIMD instructions
# - sse2: Use SSE2 instructions (most Intel/AMD processors)
# - avx2: Use AVX2 instructions (newer Intel/AMD processors)
SIMD     ?= none

# Debug options (set with make DEBUG=1)
# - 0: No debug symbols, optimized build
# - 1: Include debug symbols
DEBUG    ?= 0

# Base flags
CFLAGS_BASE := -Wall -Wextra -Werror -std=c11 -Iinclude -O2

# Add debug flags if DEBUG=1
ifeq ($(DEBUG),1)
  CFLAGS_BASE += -g -DDEBUG
endif

# SIMD flags based on choice
ifeq ($(SIMD),avx2)
  CFLAGS := $(CFLAGS_BASE) -mavx2 -DUSE_AVX2
  SIMD_SUFFIX := _avx2
else ifeq ($(SIMD),sse2)
  CFLAGS := $(CFLAGS_BASE) -msse2 -DUSE_SSE2
  SIMD_SUFFIX := _sse2
else
  CFLAGS := $(CFLAGS_BASE)
  SIMD_SUFFIX := 
endif

LDFLAGS  := -lSDL2 -lSDL2_ttf -lm -lassimp -lpthread -framework Accelerate

SRC_DIR  := src
OBJ_DIR  := build
TARGET   := renderer$(SIMD_SUFFIX)

# Find all .c files in src/
SRCS     := $(wildcard $(SRC_DIR)/*.c)
# Map src/foo.c -> build/foo.o
OBJS     := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

.PHONY: all clean debug 4x 8x run-4x run-8x

all: $(TARGET)

# Convenient target for debug build
debug:
	$(MAKE) DEBUG=1

# Convenient targets for SSE2 (4-wide SIMD) and AVX2 (8-wide SIMD)
4x:
	$(MAKE) SIMD=sse2

8x:
	$(MAKE) SIMD=avx2

run-4x: 4x
	./renderer_sse2

run-8x: 8x
	./renderer_avx2

# Link step
$(TARGET): $(OBJS)
	@echo "Linking $@"
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile step: build/foo.o from src/foo.c
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "Compiling $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure the build directory exists
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)
