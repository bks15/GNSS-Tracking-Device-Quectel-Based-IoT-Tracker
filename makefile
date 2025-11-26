.PHONY: dev build build-dev
dev: build-dev
	docker compose -f compose.dev.yaml down -t 0 --remove-orphans
	docker compose -f compose.dev.yaml up

build-dev:
	docker compose -f compose.dev.yaml build
	
build:
	docker compose build