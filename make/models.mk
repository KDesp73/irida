MODELS_DIR=data/models
HF_CLI = hf

$(MODELS_DIR):
	mkdir -p $(MODELS_DIR)

.PHONY: models.download
models.download: $(MODELS_DIR) ## Download gguf models
	$(HF_CLI) download tensorblock/Llama-3.2-1B-Instruct-GGUF \
		--include "Llama-3.2-1B-Instruct-Q3_K_M.gguf" \
		--local-dir $(MODELS_DIR)
