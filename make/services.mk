.PHONY: service.llm
service.llm: ## Start the LLM service
	python3 -m llm --serve

.PHONY: service.ui
service.ui: ## Start the website services
	cd ui && ./run
