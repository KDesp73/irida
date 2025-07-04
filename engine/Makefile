.PHONY: all
all: ## Build the engine
	zig build

.PHONY: clean
clean: ## Clean everything
	rm -rf zig-out .zig-cache

.PHONY: help
help: ## Show help message
	@echo "Available targets:"
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'

