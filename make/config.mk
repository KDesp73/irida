# Compiler and OS
UNAME_S := $(shell uname -s)
CC      := gcc
AR      := ar

# Directories
SRC_DIR      := src
INCLUDE_DIR  := include
BUILD_DIR    := build
DIST_DIR     := dist

# Library naming
LIBRARY_NAME := engine
A_NAME       := lib$(LIBRARY_NAME).a
TARGET       := $(LIBRARY_NAME)

# Version extraction
version_file := include/version.h
VERSION_MAJOR := $(shell sed -n -e 's/\#define VERSION_MAJOR \([0-9]*\)/\1/p' $(version_file))
VERSION_MINOR := $(shell sed -n -e 's/\#define VERSION_MINOR \([0-9]*\)/\1/p' $(version_file))
VERSION_PATCH := $(shell sed -n -e 's/\#define VERSION_PATCH \([0-9]*\)/\1/p' $(version_file))
VERSION       := $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)

CASTRO_VERSION=0.3.0

# macOS: use clang and platform-specific settings (BSD ld, .dylib)
ifeq ($(UNAME_S),Darwin)
    CC       := clang
    SO_NAME  := lib$(LIBRARY_NAME).dylib
    SO_LDFLAGS := -dynamiclib -undefined dynamic_lookup
    # BSD ld doesn't support -l:filename. Link both engine and castro statically (path to .a)
    # so the engine has no runtime dylib dependency and all symbols resolve at link time.
    LDFLAGS_ENGINE   := $(A_NAME)
    LDFLAGS_CASTRO   := extern/castro/libcastro.a
    LDFLAGS_NNUE_PROBE := -Lextern/nnue-probe/src -lnnueprobe -Wl,-rpath,@loader_path/extern/nnue-probe/src
    LDFLAGS_FATHOM   := -Lextern/fathom -lfathom -Wl,-rpath,@loader_path/extern/fathom
else
    SO_NAME  := lib$(LIBRARY_NAME).so
    SO_LDFLAGS := -shared
    LDFLAGS_ENGINE   := -L. -l:$(A_NAME)
    LDFLAGS_CASTRO   := -Lextern/castro -l:libcastro.a
    LDFLAGS_NNUE_PROBE := -Lextern/nnue-probe/src -lnnueprobe -Wl,-rpath,$(shell pwd)/extern/nnue-probe/src
    LDFLAGS_FATHOM   := -Lextern/fathom -lfathom -Wl,-rpath,$(shell pwd)/extern/fathom
endif

WARNINGS = -Wall -Wextra
INCLUDES = -I$(INCLUDE_DIR) -Iextern/castro/src -Iextern/ -Iextern/nnue-probe -Iextern/fathom/src

# Flags
CFLAGS  := -fPIC $(WARNINGS) $(INCLUDES)
LDFLAGS := -lpthread

# Build type
ifeq ($(type), RELEASE)
    CFLAGS += -O3
else
    SANITIZERS := -fsanitize=address,undefined
    CFLAGS  += -DDEBUG -ggdb $(SANITIZERS)
    LDFLAGS += $(SANITIZERS)
endif

# Sources
SRC_FILES := $(shell find $(SRC_DIR) -name '*.c' ! -name 'main.c')
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

TOTAL_FILES := $(words $(SRC_FILES))
counter = 0

.DEFAULT_GOAL := help

