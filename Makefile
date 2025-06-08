include common.mk
.DEFAULT_GOAL := help

SUBDIRS := movegen engine

.PHONY: all
all: ## Build every project
	@for dir in $(SUBDIRS); do \
		echo "Building $$dir..."; \
		$(MAKE) -C $$dir all || exit 1; \
	done

.PHONY: clean
clean: ## Clean every project
	@for dir in $(SUBDIRS); do \
		echo "Cleaning $$dir..."; \
		$(MAKE) -C $$dir clean || exit 1; \
	done

.PHONY: compile_commands.json
compile_commands.json: ## Generate compile_commands.json for every project
	@for dir in $(SUBDIRS); do \
		echo "Generating compile_commands.json for $$dir..."; \
		rm -f "$$dir/compile_commands.json"; \
		$(MAKE) -C "$$dir" compile_commands.json || exit 1; \
		echo "Done with $$dir."; \
	done

.PHONY: test
test: ## Build and run the tests
	make all -B
	clear
	./$(CHECK) load
	make all
	clear
	./$(CHECK)
	fastchess --compliance ./engine/engine

.PHONY: help
help: ## Show this help message
	@echo "Available commands:"
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'
