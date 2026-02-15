.PHONY: clean
clean: ## Remove all build files, libraries and the executable
	@echo "[INFO] Cleaning build artifacts"
	@rm -rf $(BUILD_DIR) $(TARGET) $(SO_NAME) $(A_NAME)

compile_commands.json: ## Generate compile commands
	@bear -- make build.all

