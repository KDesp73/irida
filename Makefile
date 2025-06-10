.DEFAULT_GOAL := help

SUBDIRS := movegen engine

.PHONY: all
all: ## Build every project
	@for dir in $(SUBDIRS); do \
		printf "\n[INFO] Building $$dir...\n"; \
		$(MAKE) -C $$dir all || exit 1; \
	done

.PHONY: clean
clean: ## Clean every project
	@for dir in $(SUBDIRS); do \
		echo "[INFO] Cleaning $$dir..."; \
		$(MAKE) -C $$dir clean || exit 1; \
	done

.PHONY: compile_commands.json
compile_commands.json: ## Generate compile_commands.json for every project
	@for dir in $(SUBDIRS); do \
		echo "[INFO] Generating compile_commands.json for $$dir..."; \
		rm -f "$$dir/compile_commands.json"; \
		$(MAKE) -C "$$dir" compile_commands.json || exit 1; \
		echo "[INFO] Done with $$dir."; \
	done

.PHONY: test_movegen
test_movegen: ## Build and run the tests
	make -C movegen all -B
	clear
	build/bin/test_movegen load
	make -C movegen all
	clear
	bin/fastchess --compliance build/bin/engine
	build/bin/test_movegen

.PHONY: test_engine
test_engine: ## Build and run the tests
	make -C engine all -B
	clear
	build/bin/test_engine load
	make -C engine all
	clear
	build/bin/test_engine

.PHONY: help
help: ## Show this help message
	@echo "Available commands:"
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'
