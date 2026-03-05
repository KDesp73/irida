.PHONY: deps.check 
deps.check: ## Check that all dependencies are available
	@command -v gcc >/dev/null 2>&1 || { echo "[ERRO] gcc missing."; exit 1; }
	@command -v bear >/dev/null 2>&1 || { echo "[WARN] bear not installed."; }

.PHONY: deps.fetch
deps.fetch: extern/castro extern/IncludeOnly nn ## Fetch dependencies

extern/castro:
	git clone https://github.com/KDesp73/castro extern/castro || echo "[INFO] already exists"
	cd extern/castro && git checkout v$(CASTRO_VERSION)

define fetchio
	curl -s -f https://raw.githubusercontent.com/KDesp73/IncludeOnly/refs/heads/main/libs/$1.h -o extern/IncludeOnly/$1.h
endef

extern/IncludeOnly:
	mkdir -p extern/IncludeOnly
	$(call fetchio,ansi)
	$(call fetchio,cli)
	$(call fetchio,logging)
	$(call fetchio,test)

nn: ## Fetch neural networks
	mkdir -p nn
	wget https://tests.stockfishchess.org/api/nn/nn-37f18f62d772.nnue -o ./nn/nn-37f18f62d772.nnue
	wget https://tests.stockfishchess.org/api/nn/nn-ad9b42354671.nnue -o ./nn/nn-ad9b42354671.nnue
