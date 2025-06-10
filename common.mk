CC = gcc

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = ../build
DIST_DIR = dist
BIN_DIR = $(BUILD_DIR)/bin

# Binaries
ENGINE = $(BIN_DIR)/engine
TEST_MOVEGEN = $(BIN_DIR)/test_movegen
TEST_ENGINE = $(BIN_DIR)/test_engine

# Sanitizers
SANITIZERS = -fsanitize=address,undefined
