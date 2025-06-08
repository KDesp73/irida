CC = gcc

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = ../build
DIST_DIR = dist
BIN_DIR = $(BUILD_DIR)/bin

# Binaries
ENGINE = $(BIN_DIR)/engine
CHECK  = $(BIN_DIR)/check

# Sanitizers
SANITIZERS = -fsanitize=address,undefined
