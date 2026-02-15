.PHONY: deps.check 
deps.check: ## Check that all dependencies are available
	@command -v gcc >/dev/null 2>&1 || { echo "[ERRO] gcc missing."; exit 1; }
	@command -v bear >/dev/null 2>&1 || { echo "[WARN] bear not installed."; }

.PHONY: deps.fetch
deps.fetch: extern/castro extern/IncludeOnly ## Fetch dependencies

extern/castro:
	git clone https://github.com/KDesp73/castro extern/castro
	cd extern/castro && git checkout v$(CASTRO_VERSION)

extern/IncludeOnly:
	mkdir -p extern/IncludeOnly
	curl https://raw.githubusercontent.com/KDesp73/IncludeOnly/refs/heads/main/libs/ansi.h -o extern/IncludeOnly/ansi.h
	curl https://raw.githubusercontent.com/KDesp73/IncludeOnly/refs/heads/main/libs/cli.h -o extern/IncludeOnly/cli.h
	curl https://raw.githubusercontent.com/KDesp73/IncludeOnly/refs/heads/main/libs/logging.h -o extern/IncludeOnly/logging.h
