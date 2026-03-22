.PHONY: deps.check deps.fetch deps.build
.PHONY: build.castro build.nnue-probe build.fathom

deps.check: ## Check that all dependencies are available
	@command -v $(CC) >/dev/null 2>&1 || { echo "[ERRO] $(CC) missing."; exit 1; }
	@command -v bear >/dev/null 2>&1 || { echo "[WARN] bear not installed."; }
	@echo "TODO: check for the availability of all dependencies. (Use script)"

# OS detection for vendor builds (nnue-probe: clang on macOS, gcc on Linux)
UNAME_S := $(shell uname -s)
NNUE_COMP := $(if $(filter Darwin,$(UNAME_S)),clang,gcc)

deps.fetch: nn vendor/IncludeOnly vendor/castro vendor/nnue-probe vendor/fathom vendor/tb ## Fetch dependency sources
	cd vendor/castro && rm -rf .git
	cd vendor/nnue-probe && rm -rf .git
	cd vendor/fathom && rm -rf .git
	cd vendor/tb && rm -rf .git

define fetchio
	curl -s -f https://raw.githubusercontent.com/KDesp73/IncludeOnly/refs/heads/main/libs/$1.h -o vendor/IncludeOnly/$1.h
endef

.PHONY: nn
deps.nn: ## Fetch halfkp_256x2-32-32-nets
	@bash ./scripts/download/nets

.PHONY: tb
deps.tb: ## Fetch lichess syzygy tablebase
	@bash ./scripts/download/tablebase

# Build targets (used by build.all and deps.build)
build.castro: vendor/castro ## Build castro move-generation library
	@echo "[INFO] Building castro"
	@[ "$(UNAME_S)" = Darwin ] && (cd vendor/castro && grep -q 'SHARED_FLAG' Makefile || (patch -N -p0 < ../../make/patches/castro-darwin.patch || true; rm -f Makefile.rej); \
		sed -e 's/-L\. -l:$$(A_NAME)/$$(A_NAME)/g' -e 's/-shared /$$(SHARED_FLAG) /g' Makefile > Makefile.tmp && mv Makefile.tmp Makefile) || true
	@cd vendor/castro && make all type=RELEASE
	@cp vendor/castro/libcastro.a 

build.nnue-probe: vendor/nnue-probe ## Build nnue-probe (Mac: clang, Linux: gcc)
	@[ "$(UNAME_S)" = Darwin ] && [ "$$(uname -m)" = arm64 ] && ( \
		cd vendor/nnue-probe/src && ( grep -q 'ifneq.*arm64' Makefile || ( \
			sed 's/\r$$//' Makefile > Makefile.tmp && mv Makefile.tmp Makefile && \
			patch -N -p0 < ../../../make/patches/nnue-probe-darwin-arm64.patch || true ); \
		grep -q 'ifneq.*arm64' Makefile || sed -e '/DEFINES += -DUSE_AVX2 -mavx2/s/^/# arm64: /' \
			-e '/DEFINES += -DUSE_SSE41 -msse4.1/s/^/# arm64: /' \
			-e '/DEFINES += -DUSE_SSE3 -msse3/s/^/# arm64: /' \
			-e '/DEFINES += -DUSE_SSE2 -msse2/s/^/# arm64: /' \
			-e '/DEFINES += -DUSE_SSE -msse$$/s/^/# arm64: /' Makefile > Makefile.tmp && mv Makefile.tmp Makefile ); \
		rm -f Makefile.rej ) || true
	@echo "[INFO] Building nnue-probe (COMP=$(NNUE_COMP))"
	@cd vendor/nnue-probe/src && make clean && \
		( [ "$(UNAME_S)" = Darwin ] && make libnnueprobe.so COMP=$(NNUE_COMP) LXXFLAGS="-lm -ldl -Wl,-install_name,@rpath/libnnueprobe.so" || make COMP=$(NNUE_COMP) )

build.fathom: vendor/fathom ## Build Fathom Syzygy library
	@echo "[INFO] Building Fathom"
	@cd vendor/fathom && \
		([ "$(UNAME_S)" = Darwin ] && make clean && make LDFLAGS="-std=gnu99 -O2 -Wall -Wshadow -Isrc -Wl,-install_name,@rpath/libfathom.so" || make)

deps.build: build.castro build.nnue-probe build.fathom ## Build all vendored dependencies

vendor/castro:
	@(git clone https://github.com/KDesp73/castro vendor/castro 2>/dev/null || true) && \
		[ -d vendor/castro ] && (cd vendor/castro && git fetch -t 2>/dev/null) || true

vendor/IncludeOnly:
	@mkdir -p vendor/IncludeOnly
	@$(call fetchio,ansi)
	@$(call fetchio,cli)
	@$(call fetchio,logging)
	@$(call fetchio,test)

vendor/nnue-probe:
	@git clone --depth=1 https://github.com/dshawul/nnue-probe.git vendor/nnue-probe || true

vendor/fathom:
	@git clone --depth=1 https://github.com/jdart1/Fathom.git vendor/fathom || true
