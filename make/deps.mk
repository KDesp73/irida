.PHONY: deps.check 
deps.check: ## Check that all dependencies are available
	@command -v $(CC) >/dev/null 2>&1 || { echo "[ERRO] $(CC) missing. On macOS install Xcode or run xcode-select --install."; exit 1; }
	@command -v bear >/dev/null 2>&1 || { echo "[WARN] bear not installed."; }

.PHONY: deps.fetch
deps.fetch: nn extern/castro extern/IncludeOnly extern/nnue-probe extern/fathom extern/Ordo extern/BayesianElo ## Fetch dependencies


define fetchio
	curl -s -f https://raw.githubusercontent.com/KDesp73/IncludeOnly/refs/heads/main/libs/$1.h -o extern/IncludeOnly/$1.h
endef

nn: ## Fetch neural networks
	mkdir -p nn
	wget https://tests.stockfishchess.org/api/nn/nn-37f18f62d772.nnue -o ./nn/nn-37f18f62d772.nnue
	wget https://tests.stockfishchess.org/api/nn/nn-ad9b42354671.nnue -o ./nn/nn-ad9b42354671.nnue
	rm ./nn-37f18f62d772.nnue
	rm ./nn-ad9b42354671.nnue

extern/castro:
	git clone https://github.com/KDesp73/castro extern/castro || echo "[INFO] already exists"
	cd extern/castro && git checkout v$(CASTRO_VERSION)

extern/IncludeOnly:
	mkdir -p extern/IncludeOnly
	$(call fetchio,ansi)
	$(call fetchio,cli)
	$(call fetchio,logging)
	$(call fetchio,test)

extern/nnue-probe:
	git clone --depth=1 https://github.com/dshawul/nnue-probe.git extern/nnue-probe
	cd extern/nnue-probe/src && make clean && make COMP=gcc

extern/fathom:
	git clone --depth=1 https://github.com/jdart1/Fathom.git extern/fathom
	cd extern/fathom && make

extern/Ordo:
	git clone --depth=1 https://github.com/michiguel/Ordo.git extern/Ordo
	cd extern/Ordo && make
	cp extern/Ordo/ordo bin

extern/BayesianElo:
	git clone --depth=1 https://github.com/ddugovic/BayesianElo.git extern/BayesianElo
	cd extern/BayesianElo/src && make
	cp extern/BayesianElo/src/bayeselo bin
