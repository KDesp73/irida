# Compiler and flags
CC = gcc
INCLUDE = -Iinclude -Ilib/raylib/include -Ilib/lua/include
CFLAGS = -Wall -Werror $(INCLUDE) -fPIC
LDFLAGS = -L./lib/raylib/lib -l:libraylib.a -lm -lpthread -ldl -L./lib/lua/lib -l:liblua.a

# Directories
SRC_DIR = src
TEST_DIR = test
INCLUDE_DIR = include
BUILD_DIR = build
DIST_DIR = dist

LIBRARY_NAME = libengine
SO_NAME = $(LIBRARY_NAME).so
A_NAME = $(LIBRARY_NAME).a

# Target and version info
EXEC = engine
CHECK = $(BUILD_DIR)/bin/check

type = DEBUG

# Determine the build type
ifeq ($(type), RELEASE) 
	CFLAGS += -O3
else
	SANITIZERS = -fsanitize=address,leak
	CFLAGS  += -DDEBUG -ggdb
	# CFLAGS  += $(SANITIZERS)
	# LDFLAGS += $(SANITIZERS)
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
all: check_tools $(BUILD_DIR) shared static check exec ## Build all libraries
	@echo "Build complete."

.PHONY: check_tools
check_tools: ## Check if necessary tools are available
	@command -v gcc >/dev/null 2>&1 || { echo >&2 "[ERRO] gcc is not installed."; exit 1; }
	@command -v bear >/dev/null 2>&1 || { echo >&2 "[WARN] bear is not installed. Skipping compile_commands.json target."; }

$(BUILD_DIR): ## Create the build directory if it doesn't exist
	@echo "[INFO] Creating build directory"
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/move
	mkdir -p $(BUILD_DIR)/bin
	mkdir -p $(BUILD_DIR)/bitboard
	mkdir -p $(BUILD_DIR)/movegen
	mkdir -p $(BUILD_DIR)/notation
	mkdir -p $(BUILD_DIR)/gui
	mkdir -p $(BUILD_DIR)/uci

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c ## Compile source files
	$(eval counter=$(shell echo $$(($(counter)+1))))
	@echo "[$(counter)/$(TOTAL_FILES)] Compiling $< -> $@"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: $(TEST_DIR)/%.c ## Compile test files
	$(eval counter=$(shell echo $$(($(counter)+1))))
	@echo "[$(counter)/$(TOTAL_FILES)] Compiling $< -> $@"
	@$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: exec
exec: $(BUILD_DIR) static ## Build executable using static library
	@echo "[INFO] Building executable: $(EXEC)"
	@$(CC) src/main.c -o $(EXEC) -L. -l:$(A_NAME) $(LDFLAGS) $(INCLUDE) -ggdb

.PHONY: check
check: $(BUILD_DIR) static ## Build the tests
	@echo "[INFO] Building test executable: $(CHECK)"
	@$(CC) $(TEST_FILES) -o $(CHECK) -L. -l:$(A_NAME) $(LDFLAGS) $(LDFLAGS) $(INCLUDE) -ggdb

.PHONY: test
test: ## Build and run the tests
	make all -B
	clear
	./$(CHECK) load
	make all
	clear
	./$(CHECK)

.PHONY: shared
shared: $(BUILD_DIR) $(OBJ_FILES) ## Build shared library
	@echo "[INFO] Building shared library: $(SO_NAME)"
	@$(CC) -shared $(CFLAGS) -o $(SO_NAME) $(OBJ_FILES)

.PHONY: static
static: $(BUILD_DIR) $(OBJ_FILES) ## Build static library
	@echo "[INFO] Building static library: $(A_NAME)"
	@$(AR) rcs $(A_NAME) $(OBJ_FILES)

.PHONY: install
install: all ## Install the executable to /usr/bin/
	@echo "[INFO] Installing $(TARGET) to /usr/bin/"
	cp $(TARGET) /usr/bin/$(TARGET)

.PHONY: uninstall
uninstall: ## Remove the executable from /usr/bin/
	@echo "[INFO] Uninstalling $(TARGET)"
	rm -f /usr/bin/$(TARGET)

.PHONY: clean
clean: ## Remove all build files and the executable
	@echo "[INFO] Cleaning up build directory and executable."
	rm -rf $(BUILD_DIR) $(TARGET) $(SO_NAME) $(A_NAME)

.PHONY: distclean
distclean: clean ## Perform a full clean, including backup and temporary files
	@echo "[INFO] Performing full clean, removing build directory, dist files, and editor backups."
	rm -f *~ core $(SRC_DIR)/*~ $(DIST_DIR)/*.tar.gz

.PHONY: dist
dist: $(SRC_FILES) ## Create a tarball of the project
	@echo "[INFO] Creating a tarball"
	mkdir -p $(DIST_DIR)
	tar -czvf $(DIST_DIR)/$(TARGET).tar.gz $(SRC_DIR) $(INCLUDE_DIR) Makefile README.md

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

## Enable verbose output for debugging
.PHONY: verbose
verbose: CFLAGS += -DVERBOSE
verbose: all ## Build the project in verbose mode

# Phony targets to avoid conflicts with file names
.PHONY: all clean distclean install uninstall dist compile_commands.json help check_tools verbose
