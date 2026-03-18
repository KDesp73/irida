.PHONY: docs docs.training docs.engine

docs: docs.training docs.engine ## Generate all docs
	rm docs/tiny.docs.json

docs.training: ## Generate documentation for the training module
	tinydocs-cli \
		--markers docs/training.tiny.markers.json \
		--dirs training \
		--ignore .gitignore \
		-o docs \
		--comment-style "#" \
		--name training
	python3 -c "import json; print(json.dumps(json.load(open('docs/tiny.docs.json')), separators=(',', ':')))" > docs/training.tiny.docs.json

docs.engine: ## Generate documentation for the engine
	tinydocs-cli \
		--dirs include \
		--markers docs/engine.tiny.markers.json \
		--ignore .gitignore \
		-o docs \
		--comment-style "//" \
		--name engine
	python3 -c "import json; print(json.dumps(json.load(open('docs/tiny.docs.json')), separators=(',', ':')))" > docs/engine.tiny.docs.json
