include ../common.mk
.DEFAULT_GOAL := help

# Change these if you use a non-system Zig or custom build options
ZIG            ?= zig            # zig executable
ZIG_TARGET     ?= native         # e.g. x86_64-linux
ZIG_OPTIMIZE   ?= Debug          # Debug / ReleaseFast / ReleaseSafe / ReleaseSmall
BUILD_ROOT     ?= .zig-cache     # Zig's default cache dir

# Extra flags for zig build
EXTRA_FLAGS    ?=

# Build command - use standard target and optimize options
BUILD_CMD  = $(ZIG) build --summary all $(EXTRA_FLAGS)
RUN_BIN    = $(ENGINE)

.PHONY: all
all: build

.PHONY: build
build: ## Build the project (zig build …)
	$(BUILD_CMD)

.PHONY: run
run: build ## Run the executable after building
	$(RUN_BIN)

.PHONY: test
test: ## Run zig test (build.zig must define "test" step for best results)
	$(ZIG) build test --summary all

.PHONY: clean
clean: ## Remove the build cache and artifacts
	@echo "Cleaning $(BUILD_ROOT) and zig-out ..."
	rm -rf $(BUILD_ROOT) zig-out

.PHONY: format
format: ## Format all Zig source files in place
	find . -name "*.zig" -exec $(ZIG) fmt {} \;

.PHONY: help
help: ## Show help message
	@echo "Available targets:"
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'
