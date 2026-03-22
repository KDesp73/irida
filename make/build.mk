.PHONY: build.all
build.all: ## Build the entire project
	@make deps.check
	@make $(BUILD_DIR)
	@make build.static
	@make build.shared
	@make $(TARGET)
	@echo "Build complete."

.PHONY: build.static
build.static: $(A_NAME) ## Build the static library

.PHONY: build.shared
build.shared: $(SO_NAME) ## Build the dynamic library
