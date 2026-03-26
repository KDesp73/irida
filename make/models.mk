MODELS_DIR=data/models
HF_CLI = hf

$(MODELS_DIR):
	mkdir -p $(MODELS_DIR)

.PHONY: models.download
models.download: $(MODELS_DIR) ## Download gguf models
	$(HF_CLI) download tensorblock/Llama-3.2-1B-Instruct-GGUF \
		--local-dir $(MODELS_DIR)
	$(HF_CLI) download ilsp/Llama-Krikri-8B-Instruct-GGUF \
		--local-dir $(MODELS_DIR)

