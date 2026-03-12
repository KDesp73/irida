.PHONY: docs 

docs: docs.training docs.engine ## Generate all docs


docs.training: ## Generate documentation for the training module
	tinydocs-cli \
		--markers docs/training.tiny.markers.json \
		--dirs training \
		--ignore .gitignore \
		-o docs \
		--comment-style "#" \
		--name engine.training
	mv docs/tiny.docs.json docs/training.tiny.docs.json

docs.engine: ## Generate documentation for the engine
	tinydocs-cli \
		--dirs include \
		--markers docs/engine.tiny.markers.json \
		--ignore .gitignore \
		-o docs \
		--comment-style "//" \
		--name engine.training
	mv docs/tiny.docs.json docs/engine.tiny.docs.json
