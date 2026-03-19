.PHONY: clean
clean: ## Remove all build files, libraries, executable and test binary
	@echo "[INFO] Cleaning build artifacts"
	@rm -rf $(BUILD_DIR) $(TARGET) $(SO_NAME) $(A_NAME) 
	@rm -rf check bench tinker nnue syzygy *.dSYM .cache

compile_commands.json: ## Generate and merge compile commands
	@bear -- make test.build
	@mv compile_commands.json test.compile_commands.json
	@bear -- make build.all type=RELEASE
	@jq -s 'add | unique' test.compile_commands.json compile_commands.json > merged.json
	@mv merged.json compile_commands.json
	@rm test.compile_commands.json
