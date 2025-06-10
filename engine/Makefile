include ../common.mk

type ?= DEBUG

LIBRARY_NAME = libengine
SO_NAME      = $(LIBRARY_NAME).so
A_NAME       = $(LIBRARY_NAME).a

TEST_DIR = test

INCLUDE  = -Iinclude -I../deps/include -I../movegen/include
CFLAGS   = -Wall -Werror $(INCLUDE) -fPIC
LDLIBS   = -l:libraylib.a -l:libmovegen.a -lm -lpthread -ldl
LDFLAGS  = -L../deps/lib -L../movegen $(LDLIBS)

ifeq ($(type), RELEASE)
	CFLAGS  += -O3
else
	CFLAGS  += -ggdb -Og $(SANITIZERS)
	LDFLAGS += $(SANITIZERS)
endif

CFLAGS += -D$(type)

SRC_FILES   := $(shell find $(SRC_DIR) -name '*.c' ! -name 'main.c')
OBJ_FILES    = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))
TEST_FILES  := $(shell find $(TEST_DIR) -name '*.c')
TOTAL_FILES := $(words $(SRC_FILES))

.DEFAULT_GOAL := help

# Targets

.PHONY: all
all: $(BUILD_DIR) shared static exec check ## Build all
	@echo "[INFO] Build complete."

$(BUILD_DIR): ## Create build directories
	@echo "[INFO] Creating build directories"
	mkdir -p $(BUILD_DIR) $(BUILD_DIR)/bin $(BUILD_DIR)/gui $(BUILD_DIR)/uci $(BUILD_DIR)/evaluation

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c ## Compile .c to .o
	@echo "[ CC ] $< -> $@"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: exec
exec: $(BUILD_DIR) static ## Build the engine executable
	@echo "[INFO] Building executable: $(ENGINE)"
	@$(CC) src/main.c -o $(ENGINE) $(INCLUDE) $(SANITIZERS) -ggdb -L. -l:$(A_NAME) $(LDFLAGS)

.PHONY: shared
shared: $(OBJ_FILES) ## Build shared library
	@echo "[INFO] Building shared library: $(SO_NAME)"
	@$(CC) -shared $(CFLAGS) -o $(SO_NAME) $(OBJ_FILES)

.PHONY: static
static: $(OBJ_FILES) ## Build static library
	@echo "[INFO] Building static library: $(A_NAME)"
	@$(AR) rcs $(A_NAME) $(OBJ_FILES)

.PHONY: check
check: $(BUILD_DIR) static ## Build test binary
	@echo "[INFO] Building test executable: $(TEST_ENGINE)"
	@$(CC) $(TEST_FILES) -o $(TEST_ENGINE) $(INCLUDE) -L. -l:$(A_NAME) $(LDFLAGS) -ggdb $(SANITIZERS)

.PHONY: clean
clean: ## Clean all generated files
	@echo "[INFO] Cleaning build artifacts"
	rm -rf $(BUILD_DIR) $(ENGINE) $(SO_NAME) $(A_NAME)

.PHONY: compile_commands.json
compile_commands.json: $(SRC_FILES) ## Generate compile_commands.json with Bear
	@echo "[INFO] Generating compile_commands.json"
	bear -- make all

.PHONY: help
help: ## Show help message
	@echo "Available targets:"
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'
