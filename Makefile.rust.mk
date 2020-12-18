all: build

build:
	cargo build

release:
	cargo build --release

test:
	cargo test

.PHONY: build
.PHONY: test
