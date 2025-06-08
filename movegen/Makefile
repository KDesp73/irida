include ../common.mk

CFLAGS = -Wall -Iinclude -fPIC
LDFLAGS =

TEST_DIR = test

LIBRARY_NAME = movegen
SO_NAME = lib$(LIBRARY_NAME).so
A_NAME = lib$(LIBRARY_NAME).a

# Target and version info
version_file = include/version.h
VERSION_MAJOR = $(shell sed -n -e 's/\#define VERSION_MAJOR \([0-9]*\)/\1/p' $(version_file))
VERSION_MINOR = $(shell sed -n -e 's/\#define VERSION_MINOR \([0-9]*\)/\1/p' $(version_file))
VERSION_PATCH = $(shell sed -n -e 's/\#define VERSION_PATCH \([0-9]*\)/\1/p' $(version_file))
VERSION = $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)

# Determine the build type
ifeq ($(type), RELEASE)
	CFLAGS += -O3
else
	SANITIZERS = # -fsanitize=address,undefined
	CFLAGS  += -DDEBUG -ggdb
	CFLAGS  += $(SANITIZERS)
	LDFLAGS += $(SANITIZERS)
endif

# Source and object files
SRC_FILES := $(shell find $(SRC_DIR) -name '*.c' ! -name 'main.c')
TEST_FILES := $(shell find $(TEST_DIR) -name '*.c')

OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))
TEST_OBJ_FILES = $(patsubst $(TEST_DIR)/%.c,$(BUILD_DIR)/%.o,$(TEST_FILES))

# Default target
.DEFAULT_GOAL := help

# Total source file count
TOTAL_FILES := $(words $(SRC_FILES))

# Counter to track progress
counter = 0

# Targets

.PHONY: all
all: check_tools $(BUILD_DIR) static shared check ## Build the project
	@echo "Build complete."

.PHONY: check_tools
check_tools: ## Check if necessary tools are available
	@command -v gcc >/dev/null 2>&1 || { echo >&2 "[ERRO] gcc is not installed."; exit 1; }
	@command -v bear >/dev/null 2>&1 || { echo >&2 "[WARN] bear is not installed. Skipping compile_commands.json target."; }

$(BUILD_DIR): ## Create the build directory if it doesn't exist
	@echo "[INFO] Creating build directory"
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/bin
	mkdir -p $(BUILD_DIR)/movegen
	mkdir -p $(BUILD_DIR)/notation
	mkdir -p $(BUILD_DIR)/bitboard

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c ## Compile source files with progress
	$(eval counter=$(shell echo $$(($(counter)+1))))
	@echo "[$(counter)/$(TOTAL_FILES)] Compiling $< -> $@"
	@$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: check
check: $(BUILD_DIR) static ## Build the tests
	@echo "[INFO] Building test executable: $(CHECK)"
	@$(CC) $(TEST_FILES) -o $(CHECK) -Iinclude -L. -l:$(A_NAME) -ggdb

.PHONY: shared
shared: $(BUILD_DIR) $(OBJ_FILES) ## Build shared library
	@echo "[INFO] Building shared library: $(SO_NAME)"
	@$(CC) -shared $(CFLAGS) -o $(SO_NAME) $(OBJ_FILES)

.PHONY: static
static: $(BUILD_DIR) $(OBJ_FILES) ## Build static library
	@echo "[INFO] Building static library: $(A_NAME)"
	@$(AR) rcs $(A_NAME) $(OBJ_FILES)

.PHONY: clean
clean: ## Remove all build files and the executable
	@echo "[INFO] Cleaning up build directory and executable."
	rm -rf $(BUILD_DIR) $(SO_NAME) $(A_NAME)

## Generate compile_commands.json
.PHONY: compile_commands.json
compile_commands.json: $(SRC_FILES) ## Generate compile_commands.json
	@echo "[INFO] Generating compile_commands.json"
	bear -- make all

## Show this help message
.PHONY: help
help: ## Show this help message
	@echo "Available commands:"
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'
