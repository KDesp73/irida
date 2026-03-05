TEST_DIR   := test
TEST_FILES := $(TEST_DIR)/main.c $(TEST_DIR)/support.c $(wildcard $(TEST_DIR)/test_*.c)
TEST_BIN   := check

.PHONY: test.generate
test.generate: ## Run loader to regenerate test registry and headers from .ctd files
	@python3 scripts/loader -d $(TEST_DIR) -H "extern/IncludeOnly/test.h" -L "castro.h"
	@echo "[INFO] Test headers synchronized."

.PHONY: test.build
test.build: build.castro build.static test.generate ## Build test binary (check)
	@echo "[INFO] Building test executable: $(TEST_BIN)"
	@$(CC) $(TEST_FILES) -o $(TEST_BIN) $(A_NAME) extern/castro/libcastro.a $(LDFLAGS) \
		-I. -I$(INCLUDE_DIR) -Iextern/castro/src -Iextern -I$(TEST_DIR) $(CFLAGS)

.PHONY: test
test: test.build ## Build and run all tests
	@./$(TEST_BIN)

.PHONY: test.batch
test.batch: test.build ## Run a single test batch: make test.batch n=1
	@./$(TEST_BIN) -n $(or $(n),1)
