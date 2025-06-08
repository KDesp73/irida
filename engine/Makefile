include ../common.mk

# Compiler and flags
INCLUDE = -Iinclude -I../deps/include -I../movegen/include 
CFLAGS = -Wall -Werror $(INCLUDE) -fPIC
LDLIBS = -l:liblua.a -l:libraylib.a -l:libmovegen.a -lm -lpthread -ldl
LDFLAGS = -L../deps/lib -L../movegen $(LDLIBS)

LIBRARY_NAME = libengine
SO_NAME = $(LIBRARY_NAME).so
A_NAME = $(LIBRARY_NAME).a

type ?= DEBUG

# Determine the build type
ifeq ($(type), RELEASE) 
	CFLAGS += -O3
else
	CFLAGS  += -ggdb -Og
	CFLAGS  += $(SANITIZERS)
	LDFLAGS += $(SANITIZERS)
endif
CFLAGS += -D$(type)


# Source and object files
SRC_FILES := $(shell find $(SRC_DIR) -name '*.c' ! -name 'main.c')
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

# Default target
.DEFAULT_GOAL := help

# Total source file count
TOTAL_FILES := $(words $(SRC_FILES))

# Counter to track progress
counter = 0

# Targets

.PHONY: all
all: $(BUILD_DIR) shared static exec ## Build all libraries
	@echo "Build complete."

$(BUILD_DIR): ## Create the build directory if it doesn't exist
	@echo "[INFO] Creating build directory"
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/bin
	mkdir -p $(BUILD_DIR)/gui
	mkdir -p $(BUILD_DIR)/uci
	mkdir -p $(BUILD_DIR)/evaluation

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c ## Compile source files
	$(eval counter=$(shell echo $$(($(counter)+1))))
	@echo "[$(counter)/$(TOTAL_FILES)] Compiling $< -> $@"
	@$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: exec
exec: $(BUILD_DIR) static ## Build executable using static library
	@echo "[INFO] Building executable: $(ENGINE)"
	$(CC) src/main.c -o $(ENGINE) $(INCLUDE) $(SANITIZERS) -ggdb -L. -l:$(A_NAME) $(LDFLAGS)

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
	@echo "[INFO] Installing $(ENGINE) to /usr/bin/"
	cp $(ENGINE) /usr/bin/$(TARGET)

.PHONY: uninstall
uninstall: ## Remove the executable from /usr/bin/
	@echo "[INFO] Uninstalling $(ENGINE)"
	rm -f /usr/bin/$(ENGINE)

.PHONY: clean
clean: ## Remove all build files and the executable
	@echo "[INFO] Cleaning up build directory and executable."
	rm -rf $(BUILD_DIR) $(ENGINE) $(SO_NAME) $(A_NAME)

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
