# NNUE training (Python). Requires: engine built, Python 3, pip.
# Override: ENGINE_PATH, FEN_FILE, DATA_CSV, EPOCHS, MODEL_OUT, MODEL_PT, MODEL_NNUE

NNUE_DIR     := training
ENGINE_PATH  ?= ./$(TARGET)
FEN_FILE     ?= $(NNUE_DIR)/positions.txt
DATA_CSV     ?= $(NNUE_DIR)/out/data.csv
EPOCHS       ?= 20
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
	python3 -m $(NNUE_DIR).generate_data --engine "$(ENGINE_PATH)" --depth 6 --fen-file "$(FEN_FILE)" --output "$(DATA_CSV)"
	@echo "[INFO] Data written to $(DATA_CSV)"

.PHONY: training.train
training.train: ## Train NNUE-style net from CSV. Override: DATA_CSV=path EPOCHS=N MODEL_OUT=path
	@test -f "$(DATA_CSV)" || (echo "Run 'make training.data' first or set DATA_CSV=..."; exit 1)
	python3 -m $(NNUE_DIR).train --data "$(DATA_CSV)" --epochs "$(EPOCHS)" --output "$(MODEL_OUT)"
	@echo "[INFO] Model saved to $(MODEL_OUT)"

.PHONY: training.convert
training.convert: ## Convert .pt to .nnue. Override: MODEL_PT=path MODEL_NNUE=path
	@test -f "$(MODEL_PT)" || (echo "No $(MODEL_PT). Run 'make training.train' first or set MODEL_PT=..."; exit 1)
	python3 -m $(NNUE_DIR).convert_pt_to_nnue "$(MODEL_PT)" "$(MODEL_NNUE)"
	@echo "[INFO] Set UCI EvalFile to $(MODEL_NNUE) to use in engine."

.PHONY: training.clean
training.clean: ## Remove resulting files
	rm -rf $(NNUE_DIR)/out
	@echo "[INFO] NNUE training artifacts removed"

.PHONY: training.all
training.all: training.data training.train training.convert
