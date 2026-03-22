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

ifeq ($(UNAME_S),Darwin) # macOS
    CC       := clang
    SO_NAME  := lib$(LIBRARY_NAME).dylib
    SO_LDFLAGS := -dynamiclib -undefined dynamic_lookup
    LDFLAGS_ENGINE   := $(A_NAME)
    LDFLAGS_CASTRO   := vendor/castro/libcastro.a
    LDFLAGS_NNUE_PROBE := vendor/nnue-probe/src/libnnueprobe.a
    LDFLAGS_FATHOM   := vendor/fathom/libfathom.a
else
    SO_NAME  := lib$(LIBRARY_NAME).so
    SO_LDFLAGS := -shared
    LDFLAGS_ENGINE   := -L. -l:$(A_NAME)
    LDFLAGS_CASTRO   := -Lvendor/castro -l:libcastro.a
	LDFLAGS_NNUE_PROBE := -Lvendor/nnue-probe/src -l:libnnueprobe.a
	LDFLAGS_FATHOM   := -Lvendor/fathom -l:libfathom.a
endif

WARNINGS = -Wall -Wextra
INCLUDES = -I$(INCLUDE_DIR) -Ivendor/castro/src -Ivendor -Ivendor/nnue-probe -Ivendor/fathom/src

# Flags
CFLAGS  := -fPIC $(WARNINGS) $(INCLUDES) -DUSE_FATHOM
LDFLAGS := -lpthread $(LDFLAGS_FATHOM) $(LDFLAGS_NNUE_PROBE)

type := DEBUG

# Build type
ifeq ($(type), RELEASE)
    CFLAGS += -O3
else
    SANITIZERS := -fsanitize=address,undefined
    CFLAGS  += -DDEBUG -ggdb $(SANITIZERS)
    LDFLAGS += $(SANITIZERS)
endif

# Sources
SRC_FILES := $(shell find $(SRC_DIR) -name '*.c')
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

.DEFAULT_GOAL := help
