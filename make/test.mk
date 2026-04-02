TEST_DIR   := test
TEST_FILES := apps/test.c $(wildcard $(TEST_DIR)/test_*.c)
TEST_BIN   := check

.PHONY: test.generate
test.generate: ## Run loader to regenerate test registry and headers from .ctd files
	@python3 scripts/loader -d $(TEST_DIR) -H "IncludeOnly/test.h" -L "castro.h"
	@echo "[INFO] Test headers synchronized."

.PHONY: test.build
test.build: build.static test.generate ## Build test binary (check)
	@echo "[INFO] Building test executable: $(TEST_BIN)"
	@$(CC) $(CFLAGS) $(TEST_FILES) -o $(TEST_BIN) $(LDFLAGS_ENGINE) $(LDFLAGS_CASTRO) $(LDFLAGS) \
		-I. -I$(INCLUDE_DIR) -Ideps/include -I$(TEST_DIR) 

.PHONY: test
test: test.build ## Build and run all tests
	@./$(TEST_BIN)

# Gauntlet (engine-vs-engine strength / regression testing). Requires cutechess-cli.
# Optional: REF_ENGINE=/path/to/engine GAMES=50 TC=10+0.1. Build with type=RELEASE for meaningful results.
REF_ENGINE ?=
GAMES ?= 50
TC ?= 10+0.1

.PHONY: gauntlet
test.gauntlet: $(TARGET) ## Run gauntlet (cutechess-cli). Optional: REF_ENGINE=path GAMES=N TC=10+0.1
	@./scripts/run-gauntlet $(REF_ENGINE) $(GAMES) $(TC)

.PHONY: test.engine
test.engine: test.build $(TARGET) ## Unit tests plus one fixed-depth search (sanity check)
	@./$(TEST_BIN)
	@echo "[INFO] Running search sanity check (depth 2)..."
	@(echo "uci"; echo "position startpos"; echo "go depth 2"; echo "quit") | ./$(TARGET) > /dev/null && echo "[INFO] Engine search OK."
