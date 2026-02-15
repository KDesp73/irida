DIST_INCLUDE = $(SRC_DIR) $(INCLUDE_DIR) $(SO_NAME) $(A_NAME) $(TARGET)

.PHONY: dist
dist: $(DIST_INCLUDE) ## Create a tarball of the project
	@echo "[INFO] Creating release $(VERSION)"
	@mkdir -p $(DIST_DIR)
	@tar -czvf $(DIST_DIR)/$(TARGET)-$(VERSION).tar.gz \
		$(DIST_INCLUDE)


.PHONY: dist.clean
dist.clean: ## Perform a full dist clean
	@echo "[INFO] Removing dist files"
	@rm -rf $(DIST_DIR)

