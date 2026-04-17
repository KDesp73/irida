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
LIBRARY_NAME := irida
A_NAME       := lib$(LIBRARY_NAME).a
TARGET       := $(LIBRARY_NAME)

# Version extraction
version_file := include/version.h
VERSION_MAJOR := $(shell sed -n -e 's/\#define VERSION_MAJOR \([0-9]*\)/\1/p' $(version_file))
VERSION_MINOR := $(shell sed -n -e 's/\#define VERSION_MINOR \([0-9]*\)/\1/p' $(version_file))
VERSION_PATCH := $(shell sed -n -e 's/\#define VERSION_PATCH \([0-9]*\)/\1/p' $(version_file))
VERSION       := $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)

LDFLAGS_NNUE_PROBE := -lnnueprobe
LDFLAGS_FATHOM     := -lfathom

ifeq ($(UNAME_S),Darwin) # macOS
    CC       := clang
    SO_NAME  := lib$(LIBRARY_NAME).dylib
    SO_LDFLAGS := -dynamiclib -undefined dynamic_lookup
	LDFLAGS := -L./deps/lib/macos -Wl,-rpath,$(shell pwd)/deps/lib/macos
	LDFLAGS_ENGINE := $(A_NAME)
	LDFLAGS_CASTRO := deps/lib/macos/libcastro.a
else
    SO_NAME  := lib$(LIBRARY_NAME).so
    SO_LDFLAGS := -shared
	LDFLAGS := -L./deps/lib/linux -Wl,-rpath,$(shell pwd)/deps/lib/linux
	LDFLAGS_ENGINE := -l:$(A_NAME)
	LDFLAGS_CASTRO := -l:libcastro.a
endif

WARNINGS = -Wall -Wextra
INCLUDES = -I$(INCLUDE_DIR) -Ideps/include -Ideps/include/fathom -Ideps/include/nnue-probe

# Flags
CFLAGS  := -fPIC $(WARNINGS) $(INCLUDES)
LDFLAGS += -lpthread -L. $(LDFLAGS_CASTRO) $(LDFLAGS_FATHOM) $(LDFLAGS_NNUE_PROBE) 
type := RELEASE

# Set NATIVE=1 for -march=native (local max speed; less reproducible across CPUs).
NATIVE ?= 0

# Build type
ifeq ($(type), RELEASE)
    CFLAGS += -O3
    ifeq ($(NATIVE),1)
        CFLAGS += -march=native
    endif
else
    SANITIZERS := -fsanitize=address,undefined
    CFLAGS  += -DDEBUG -ggdb $(SANITIZERS)
    LDFLAGS += $(SANITIZERS)
endif

# Sources
SRC_FILES := $(shell find $(SRC_DIR) -name '*.c')
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

.DEFAULT_GOAL := help
