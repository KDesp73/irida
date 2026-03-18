# NNUE training (Python). Requires: engine built, Python 3, pip.
# Override: ENGINE_PATH, FEN_FILE, DATA_CSV, EPOCHS, MODEL_OUT, MODEL_PT, MODEL_NNUE

NNUE_DIR     := training
ENGINE_PATH  ?= ./$(TARGET)
FEN_FILE     ?= $(NNUE_DIR)/positions.txt
DATA_CSV     ?= $(NNUE_DIR)/out/data.csv
EPOCHS       ?= 6
MODEL_OUT    ?= $(NNUE_DIR)/out/model.pt
MODEL_PT     ?= $(NNUE_DIR)/out/model.pt
MODEL_NNUE   ?= $(NNUE_DIR)/out/model.nnue

.PHONY: training.deps
training.deps: ## Install Python deps for NNUE training (pip install torch)
	 pip install torch

.PHONY: training.data
training.data: $(TARGET) ## Generate training CSV from engine. Override: FEN_FILE=path DATA_CSV=out.csv ENGINE_PATH=./engine
	@test -f "$(FEN_FILE)" || (echo "Create $(FEN_FILE) with one FEN per line, or set FEN_FILE=..."; exit 1)
	mkdir -p $(NNUE_DIR)/out
	python3 -m $(NNUE_DIR) data --engine "stockfish" --depth 6 --fen-file "$(FEN_FILE)" --output "$(DATA_CSV)"
	@echo "[INFO] Data written to $(DATA_CSV)"

.PHONY: training.train
training.train: ## Train NNUE-style net from CSV. Override: DATA_CSV=path EPOCHS=N MODEL_OUT=path
	@test -f "$(DATA_CSV)" || (echo "Run 'make training.data' first or set DATA_CSV=..."; exit 1)
	python3 -m $(NNUE_DIR) train --data "$(DATA_CSV)" --epochs "$(EPOCHS)" --output "$(MODEL_OUT)" --arch halfkp
	@echo "[INFO] Model saved to $(MODEL_OUT)"

.PHONY: training.convert
training.convert: ## Convert .pt to .nnue. Override: MODEL_PT=path MODEL_NNUE=path
	@test -f "$(MODEL_PT)" || (echo "No $(MODEL_PT). Run 'make training.train' first or set MODEL_PT=..."; exit 1)
	python3 -m $(NNUE_DIR) convert "$(MODEL_PT)" "$(MODEL_NNUE)"
	@echo "[INFO] Set UCI EvalFile to $(MODEL_NNUE) to use in engine."

.PHONY: training.clean
training.clean: ## Remove resulting files
	rm -rf $(NNUE_DIR)/out
	@echo "[INFO] NNUE training artifacts removed"

.PHONY: training.all
training.all: training.data training.train training.convert

# Texel tuning: tune PeSTO piece values (mg_value, eg_value) from positions with results.
TEXEL_CSV    ?= $(NNUE_DIR)/out/texel_positions.csv
TEXEL_PARAMS ?= $(NNUE_DIR)/out/texel_params.json
TEXEL_PGN    ?= $(NNUE_DIR)/games.pgn

.PHONY: training.texel.deps
training.texel.deps: ## Install deps for Texel (numpy, scipy; optional: python-chess for PGN)
	pip install numpy scipy

.PHONY: training.texel.data
training.texel.data: ## Build fen,result CSV from PGN. Override: TEXEL_PGN=file.pgn TEXEL_CSV=out.csv
	@test -f "$(TEXEL_PGN)" || (echo "Set TEXEL_PGN=path/to/games.pgn"; exit 1)
	mkdir -p $(NNUE_DIR)/out
	python3 -m $(NNUE_DIR) pgn2texel --pgn "$(TEXEL_PGN)" --output "$(TEXEL_CSV)"
	@echo "[INFO] Texel dataset: $(TEXEL_CSV)"

.PHONY: training.texel.tune
training.texel.tune: $(TARGET) ## Run Texel tuning. Override: TEXEL_CSV=... TEXEL_PARAMS=... ENGINE_PATH=./engine
	@test -f "$(TEXEL_CSV)" || (echo "Run 'make training.texel.data' or set TEXEL_CSV=..."; exit 1)
	python3 -m $(NNUE_DIR) texel --data "$(TEXEL_CSV)" --output "$(TEXEL_PARAMS)" --iter 1000 --engine "$(ENGINE_PATH)"
	@echo "[INFO] Tuned params: $(TEXEL_PARAMS). Paste the C snippet into src/eval/pesto.c"

.PHONY: training.texel.tune_weights
training.texel.tune_weights: $(TARGET) ## Tune PeSTO term weights from breakdown. Override: TEXEL_CSV=... ENGINE_PATH=...
	@test -f "$(TEXEL_CSV)" || (echo "Run 'make training.texel.data' or set TEXEL_CSV=..."; exit 1)
	python3 -m $(NNUE_DIR) texel-weights --data "$(TEXEL_CSV)" --engine "$(ENGINE_PATH)" --output "$(NNUE_DIR)/out/texel_weights.json" --iter 500
	@echo "[INFO] Tuned weights: $(NNUE_DIR)/out/texel_weights.json"
