.PHONY: build.all
build.all: deps.check deps.build $(BUILD_DIR) build.static build.shared $(TARGET) bench ## Build the entire project
	@echo "Build complete."

.PHONY: build.static
build.static: $(A_NAME) ## Build the static library

.PHONY: build.shared
build.shared: $(SO_NAME) ## Build the dynamic library
