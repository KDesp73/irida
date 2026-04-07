$(BUILD_DIR):
	@echo "[INFO] Creating build directory"
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "[CC] $< -> $@"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c -o $@ $< 

$(A_NAME): $(BUILD_DIR) $(OBJ_FILES)
	@echo "[INFO] Building static library: $(A_NAME)"
	@$(AR) rcs $(A_NAME) $(OBJ_FILES)

$(SO_NAME): $(BUILD_DIR) $(OBJ_FILES) ## Build the dynamic library
	@echo "[INFO] Building shared library: $(SO_NAME)"
	@$(CC) $(SO_LDFLAGS) -o $(SO_NAME) $(OBJ_FILES)

%: apps/%.c apps/shared.h $(A_NAME)
	@echo "[INFO] Building executable: $@"
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS_ENGINE) $(LDFLAGS_CASTRO) $(LDFLAGS) -DENABLE_ALL


.PHONY: irida-plain
irida-plain: apps/irida.c apps/shared.h $(A_NAME) ## Only move ordering, alpha-beta pruning and iterative deepening
	@echo "[INFO] Building executable: $@"
	$(CC) $(CFLAGS) -DIRIDA_UCI_NAME=\"$@\" $< -o $@ \
		$(LDFLAGS_ENGINE) $(LDFLAGS_CASTRO) $(LDFLAGS)

.PHONY: irida-q
irida-q: apps/irida.c apps/shared.h $(A_NAME) ## irida-plain + quiescence
	@echo "[INFO] Building executable: $@"
	$(CC) $(CFLAGS) -DENABLE_QUIESCENCE -DIRIDA_UCI_NAME=\"$@\" $< -o $@ \
		$(LDFLAGS_ENGINE) $(LDFLAGS_CASTRO) $(LDFLAGS)

.PHONY: irida-q-tt
irida-q-tt: apps/irida.c apps/shared.h $(A_NAME) ## irida-q + tt
	@echo "[INFO] Building executable: $@"
	$(CC) $(CFLAGS) \
		-DENABLE_QUIESCENCE \
		-DENABLE_TT \
		-DIRIDA_UCI_NAME=\"$@\" $< -o $@ \
		$(LDFLAGS_ENGINE) $(LDFLAGS_CASTRO) $(LDFLAGS)

.PHONY: irida-q-tt-nmp
irida-q-tt-nmp: apps/irida.c apps/shared.h $(A_NAME) ## irida-q-tt + nmp
	@echo "[INFO] Building executable: $@"
	$(CC) $(CFLAGS) \
		-DENABLE_QUIESCENCE \
		-DENABLE_TT \
		-DENABLE_NMP \
		-DIRIDA_UCI_NAME=\"$@\" $< -o $@ \
		$(LDFLAGS_ENGINE) $(LDFLAGS_CASTRO) $(LDFLAGS)

.PHONY: irida-q-tt-nmp-lmr
irida-q-tt-nmp-lmr: apps/irida.c apps/shared.h $(A_NAME) ## irida-q-tt-nmp + lmr
	@echo "[INFO] Building executable: $@"
	$(CC) $(CFLAGS) \
		-DENABLE_QUIESCENCE \
		-DENABLE_TT \
		-DENABLE_NMP \
		-DENABLE_LMR \
		-DIRIDA_UCI_NAME=\"$@\" $< -o $@ \
		$(LDFLAGS_ENGINE) $(LDFLAGS_CASTRO) $(LDFLAGS)

.PHONY: irida-q-tt-nmp-lmr-syzygy
irida-q-tt-nmp-lmr-syzygy: apps/irida.c apps/shared.h $(A_NAME) ## irida-q-tt-nmp-lmr + syzygy
	@echo "[INFO] Building executable: $@"
	$(CC) $(CFLAGS) \
		-DENABLE_QUIESCENCE \
		-DENABLE_TT \
		-DENABLE_NMP \
		-DENABLE_LMR \
		-DENABLE_SYZYGY \
		-DIRIDA_UCI_NAME=\"$@\" $< -o $@ \
		$(LDFLAGS_ENGINE) $(LDFLAGS_CASTRO) $(LDFLAGS)

.PHONY: irida-q-tt-nmp-lmr-syzygy-aw
irida-q-tt-nmp-lmr-syzygy-aw: apps/irida.c apps/shared.h $(A_NAME) ## irida-q-tt-nmp-lmr-syzygy + aw
	@echo "[INFO] Building executable: $@"
	$(CC) $(CFLAGS) \
		-DENABLE_QUIESCENCE \
		-DENABLE_TT \
		-DENABLE_NMP \
		-DENABLE_LMR \
		-DENABLE_SYZYGY \
		-DENABLE_ASPIRATION \
		-DIRIDA_UCI_NAME=\"$@\" $< -o $@ \
		$(LDFLAGS_ENGINE) $(LDFLAGS_CASTRO) $(LDFLAGS)

.PHONY: irida-q-tt-nmp-lmr-syzygy-aw-pvs
irida-q-tt-nmp-lmr-syzygy-aw-pvs: irida ## irida-q-tt-nmp-lmr-syzygy-aw + pvs. Same as irida
	cp irida $@

.PHONY: ab-testing-targets
ab-testing-targets: ## Make all A/B Testing targets
	make irida-plain
	make irida-q
	make irida-q-tt
	make irida-q-tt-nmp
	make irida-q-tt-nmp-lmr
	make irida-q-tt-nmp-lmr-syzygy
	make irida-q-tt-nmp-lmr-syzygy-aw
	make irida-q-tt-nmp-lmr-syzygy-aw-pvs
