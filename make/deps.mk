.PHONY: deps.check 
deps.check: ## Check that all dependencies are available
	@command -v gcc >/dev/null 2>&1 || { echo "[ERRO] gcc missing."; exit 1; }
	@command -v bear >/dev/null 2>&1 || { echo "[WARN] bear not installed."; }

