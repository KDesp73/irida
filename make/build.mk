.PHONY: build.all
build.all: deps.check $(BUILD_DIR) build.castro build.static build.shared $(TARGET) ## Build the entire project
	@echo "Build complete."

$(TARGET): build.static ## Build the main executable
	@echo "[INFO] Building executable: $(TARGET)"
	$(CC) $(CFLAGS) src/main.c -o $(TARGET) $(LDFLAGS_ENGINE) $(LDFLAGS_CASTRO) $(LDFLAGS) 

.PHONY: build.static
build.static: $(BUILD_DIR) $(OBJ_FILES) ## Build the static library
	@echo "[INFO] Building static library: $(A_NAME)"
	@$(AR) rcs $(A_NAME) $(OBJ_FILES)

.PHONY: build.shared
build.shared: $(BUILD_DIR) $(OBJ_FILES) ## Build the dynamic library
	@echo "[INFO] Building shared library: $(SO_NAME)"
	@$(CC) $(SO_LDFLAGS) -o $(SO_NAME) $(OBJ_FILES)

tinker: build.static ## Build the tinker executable
	@echo "[INFO] Building executable: tinker"
	$(CC) $(CFLAGS) src/tinker.c -o tinker $(LDFLAGS_ENGINE) $(LDFLAGS_CASTRO) $(LDFLAGS) 

benchmark: build.static
	@echo "[INFO] Building executable: benchmark"
	$(CC) $(CFLAGS) bench/main.c -o benchmark $(LDFLAGS_ENGINE) $(LDFLAGS_CASTRO) $(LDFLAGS) 

.PHONY: build.verbose
build.verbose: CFLAGS += -DVERBOSE
build.verbose: all ## Build the project in verbose mode
