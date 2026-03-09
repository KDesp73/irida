.PHONY: deps.check deps.fetch deps.build
.PHONY: build.castro build.nnue-probe build.fathom

deps.check: ## Check that all dependencies are available
	@command -v $(CC) >/dev/null 2>&1 || { echo "[ERRO] $(CC) missing."; exit 1; }
	@command -v bear >/dev/null 2>&1 || { echo "[WARN] bear not installed."; }

# OS detection for vendor builds (nnue-probe: clang on macOS, gcc on Linux)
UNAME_S := $(shell uname -s)
NNUE_COMP := $(if $(filter Darwin,$(UNAME_S)),clang,gcc)

deps.fetch: nn vendor/castro vendor/IncludeOnly vendor/nnue-probe vendor/fathom ## Fetch dependency sources

define fetchio
	curl -s -f https://raw.githubusercontent.com/KDesp73/IncludeOnly/refs/heads/main/libs/$1.h -o vendor/IncludeOnly/$1.h
endef

# Neural network files (Stockfish .nnue). Use -O to write to file; -o is log file for wget.
nn:
	@mkdir -p nn
	@curl -sSfL -o nn/nn-37f18f62d772.nnue https://tests.stockfishchess.org/api/nn/nn-37f18f62d772.nnue
	@curl -sSfL -o nn/nn-ad9b42354671.nnue https://tests.stockfishchess.org/api/nn/nn-ad9b42354671.nnue

vendor/castro:
	@(git clone https://github.com/KDesp73/castro vendor/castro 2>/dev/null || true) && \
		cd vendor/castro && git checkout v$(CASTRO_VERSION)

vendor/IncludeOnly:
	@mkdir -p vendor/IncludeOnly
	@$(call fetchio,ansi)
	@$(call fetchio,cli)
	@$(call fetchio,logging)
	@$(call fetchio,test)

vendor/nnue-probe:
	@git clone --depth=1 https://github.com/dshawul/nnue-probe.git vendor/nnue-probe

vendor/fathom:
	@git clone --depth=1 https://github.com/jdart1/Fathom.git vendor/fathom

# Build targets (used by build.all and deps.build)
build.castro: vendor/castro ## Build castro move-generation library
	@echo "[INFO] Building castro"
	@cd vendor/castro && make all type=RELEASE

build.nnue-probe: vendor/nnue-probe ## Build nnue-probe (Mac: clang, Linux: gcc)
	@[ "$(UNAME_S)" = Darwin ] && [ "$$(uname -m)" = arm64 ] && ( \
		cd vendor/nnue-probe/src && grep -q 'ifneq.*arm64' Makefile || ( \
			sed 's/\r$$//' Makefile > Makefile.tmp && mv Makefile.tmp Makefile && \
			patch -N -p0 < ../../../make/patches/nnue-probe-darwin-arm64.patch ); \
		rm -f Makefile.rej ) || true
	@echo "[INFO] Building nnue-probe (COMP=$(NNUE_COMP))"
	@cd vendor/nnue-probe/src && make clean && \
		( [ "$(UNAME_S)" = Darwin ] && make libnnueprobe.so COMP=$(NNUE_COMP) LXXFLAGS="-lm -ldl -Wl,-install_name,@rpath/libnnueprobe.so" || make COMP=$(NNUE_COMP) )

build.fathom: vendor/fathom ## Build Fathom Syzygy library
	@echo "[INFO] Building Fathom"
	@cd vendor/fathom && \
		([ "$(UNAME_S)" = Darwin ] && make clean && make LDFLAGS="-std=gnu99 -O2 -Wall -Wshadow -Isrc -Wl,-install_name,@rpath/libfathom.so" || make)

deps.build: build.castro build.nnue-probe build.fathom ## Build all vendored dependencies

# Optional tools (Ordo, BayesianElo) — not required for engine build
vendor/Ordo:
	@git clone --depth=1 https://github.com/michiguel/Ordo.git vendor/Ordo
	@cd vendor/Ordo && make
	@mkdir -p bin && cp vendor/Ordo/ordo bin/

vendor/BayesianElo:
	@git clone --depth=1 https://github.com/ddugovic/BayesianElo.git vendor/BayesianElo
	@cd vendor/BayesianElo/src && make
	@mkdir -p bin && cp vendor/BayesianElo/src/bayeselo bin/
