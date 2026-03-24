LIB_DIR = /usr/local/lib
BIN_DIR = /usr/local/bin
IRIDA_DAT_DIR = ${HOME}/.irida
IRIDA_INC_DIR = /usr/local/include/irida

.PHONY: install
install: $(TARGET) install.deps install.lib install.data ## Installs irida executable, library and dependencies
	@echo "Installing irida executable"
	@sudo cp $(TARGET) $(BIN_DIR)

.PHONY: install.lib
install.lib: $(A_NAME) $(SO_NAME) ## Installs the irida library (libirida.* and *.h files)
	@echo "Installing irida library"
	@sudo cp $(A_NAME) $(SO_NAME) $(LIB_DIR)
	@sudo mkdir -p $(IRIDA_INC_DIR)
	@sudo cp include/* $(IRIDA_INC_DIR)

.PHONY: install.deps
install.deps: ## Installs irida's dependencies
	@echo "Installing dependencies in $(LIB_DIR)"
	@sudo cp deps/lib/libnnueprobe.so $(LIB_DIR)
	@sudo cp deps/lib/libfathom.so $(LIB_DIR)

.PHONY: install.data
install.data: ## Installs necessary data to ~/.irida
	@echo "Installing data in ~/.irida"
	@mkdir -p $(IRIDA_DAT_DIR)
	@cp -r data/nn data/tb $(IRIDA_DAT_DIR)

.PHONY: uninstall
uninstall: ## Removes all installed artifacts 
	@echo "Uninstalling irida"
	sudo rm -rf $(IRIDA_DAT_DIR) $(IRIDA_INC_DIR)
	sudo rm $(LIB_DIR)/libnnueprobe.so
	sudo rm $(LIB_DIR)/libfathom.so
	sudo rm $(LIB_DIR)/libirida.so
	sudo rm $(LIB_DIR)/libirida.a
	sudo rm $(BIN_DIR)/$(TARGET)
