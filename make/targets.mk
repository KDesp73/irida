$(BUILD_DIR):
	@echo "[INFO] Creating build directory"
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "[CC] $< -> $@"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c -o $@ $< 

$(A_NAME): $(BUILD_DIR) $(OBJ_FILES)
	@echo "[INFO] Building static library: $(A_NAME)"
	@$(AR) rcs $(A_NAME) $(OBJ_FILES)

$(SO_NAME): $(BUILD_DIR) $(OBJ_FILES) ## Build the dynamic library
	@echo "[INFO] Building shared library: $(SO_NAME)"
	@$(CC) $(SO_LDFLAGS) -o $(SO_NAME) $(OBJ_FILES)

%: apps/%.c apps/shared.h $(A_NAME)
	@echo "[INFO] Building executable: $@"
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS_ENGINE) $(LDFLAGS_CASTRO) $(LDFLAGS)

# Distinct UCI id for A/B matches (arena/PGN). Pair with shared.h changes (e.g. usePVS).
.PHONY: irida-no-pvs
irida-no-pvs: apps/irida.c apps/shared.h $(A_NAME) ## Same as irida but id name irida-no-pvs
	@echo "[INFO] Building executable: irida-no-pvs (IRIDA_UCI_NAME=irida-no-pvs)"
	@$(CC) $(CFLAGS) -DIRIDA_UCI_NAME=\"irida-no-pvs\" apps/irida.c -o irida-no-pvs \
		$(LDFLAGS_ENGINE) $(LDFLAGS_CASTRO) $(LDFLAGS)
