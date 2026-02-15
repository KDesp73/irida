.PHONY: build.all
build.all: deps.check $(BUILD_DIR) build.castro build.static build.shared $(TARGET) ## Build the entire project
	@echo "Build complete."

$(TARGET): build.static ## Build the main executable
	@echo "[INFO] Building executable: $(TARGET)"
	$(CC) src/main.c -o $(TARGET) -L. -l:$(A_NAME) $(LDFLAGS) -I$(INCLUDE_DIR) -Iextern/castro/src

.PHONY: build.static
build.static: $(BUILD_DIR) $(OBJ_FILES) ## Build the static library
	@echo "[INFO] Building static library: $(A_NAME)"
	@$(AR) rcs $(A_NAME) $(OBJ_FILES)

.PHONY: build.shared
build.shared: $(BUILD_DIR) $(OBJ_FILES) ## Build the dynamic library
	@echo "[INFO] Building shared library: $(SO_NAME)"
	@$(CC) -shared -o $(SO_NAME) $(OBJ_FILES)

.PHONY: build.verbose
build.verbose: CFLAGS += -DVERBOSE
build.verbose: all ## Build the project in verbose mode

.PHONY: build.castro
build.castro: extern/castro ## Build the move generation library
	cd extern/castro && make all
