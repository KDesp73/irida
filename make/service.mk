# Docker: Irida (host mount) + Stockfish (image). See docker-compose.yml.
DOCKER_COMPOSE ?= docker compose
COMPOSE := -f docker-compose.yml

HOST_PORT ?= 8765
IRIDA_ENGINE_PATH ?= ./irida
IRIDA_LIBS_PATH ?= ./deps/lib/linux

.PHONY: docker.build docker.up docker.down docker.logs docker.package.artifacts

docker.build: ## Build the web service image
	$(DOCKER_COMPOSE) $(COMPOSE) build

docker.up: ## Run UI + API (Irida + Stockfish; set HOST_PORT / IRIDA_* paths as needed)
	HOST_PORT=$(HOST_PORT) IRIDA_ENGINE_PATH=$(IRIDA_ENGINE_PATH) IRIDA_LIBS_PATH=$(IRIDA_LIBS_PATH) \
		$(DOCKER_COMPOSE) $(COMPOSE) up --build

docker.down: ## Stop the stack
	$(DOCKER_COMPOSE) $(COMPOSE) down

docker.logs: ## Follow container logs
	$(DOCKER_COMPOSE) $(COMPOSE) logs -f

docker.package.artifacts: irida ## Copy irida + .so into artifacts/
	@mkdir -p artifacts/deps/lib/linux
	@cp -f irida artifacts/
	@cp -f deps/lib/linux/*.so artifacts/deps/lib/linux/
	@echo "Wrote artifacts/irida and artifacts/deps/lib/linux/*.so"
