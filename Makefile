include make/config.mk
include make/rules.mk
include make/utils.mk
include make/deps.mk
include make/dist.mk
include make/build.mk
include make/test.mk
include make/training.mk

.PHONY: help
help: ## Show this help message
	@echo "Available commands:"
	@grep -h -E '^[a-zA-Z0-9_.-]+:.*?## .*$$' $(MAKEFILE_LIST) | \
		awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-25s\033[0m %s\n", $$1, $$2}'
