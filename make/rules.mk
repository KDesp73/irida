$(BUILD_DIR):
	@echo "[INFO] Creating build directory"
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(eval counter=$(shell echo $$(($(counter)+1))))
	@echo "[$(counter)/$(TOTAL_FILES)] Compiling $< -> $@"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c -o $@ $<
