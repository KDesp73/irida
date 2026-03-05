.PHONY: clean
clean: ## Remove all build files, libraries, executable and test binary
	@echo "[INFO] Cleaning build artifacts"
	@rm -rf $(BUILD_DIR) $(TARGET) $(SO_NAME) $(A_NAME) check

compile_commands.json: ## Generate compile commands
	@bear -- make build.all
