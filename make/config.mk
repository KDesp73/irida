# Compiler
CC       := gcc
AR       := ar

# Directories
SRC_DIR      := src
INCLUDE_DIR  := include
BUILD_DIR    := build
DIST_DIR     := dist

# Library naming
LIBRARY_NAME := engine
SO_NAME      := lib$(LIBRARY_NAME).so
A_NAME       := lib$(LIBRARY_NAME).a
TARGET       := engine

# Version extraction
version_file := include/version.h
VERSION_MAJOR := $(shell sed -n -e 's/\#define VERSION_MAJOR \([0-9]*\)/\1/p' $(version_file))
VERSION_MINOR := $(shell sed -n -e 's/\#define VERSION_MINOR \([0-9]*\)/\1/p' $(version_file))
VERSION_PATCH := $(shell sed -n -e 's/\#define VERSION_PATCH \([0-9]*\)/\1/p' $(version_file))
VERSION       := $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)

# Flags
CFLAGS  := -Wall -Wextra -fPIC -I$(INCLUDE_DIR) -Iextern/castro/src
LDFLAGS := -Lextern/castro -l:libcastro.a

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

