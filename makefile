include versions.env
export

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
PLATFORM ?= macos
else
PLATFORM ?= linux
endif
export PLATFORM

.PHONY: all static shared wasm wasi docker-linux docker-windows \
	wasi/run wasm/server install/wasmtime clean format

all: static

static:
	LINKAGE=static bash scripts/cmake-build.sh

shared:
	LINKAGE=shared bash scripts/cmake-build.sh

wasm:
	bash scripts/wasm-build.sh

wasi:
	PLATFORM=wasi bash scripts/cmake-build.sh

docker-linux:
	bash scripts/docker-build.sh linux

docker-linux-shared:
	bash scripts/docker-build.sh linux shared

docker-windows:
	bash scripts/docker-build.sh windows

docker-windows-shared:
	bash scripts/docker-build.sh windows shared

wasi/run:
	wasmtime --dir=. dist/wasi-wasm32-static/bgdc.wasm -- -o web/demo/hello.dcb web/demo/hello.prg

install/wasmtime:
	curl https://wasmtime.dev/install.sh -sSf | bash

wasm/server:
	python3 -m http.server 8080 --directory dist/web-wasm32-static

clean:
	rm -rf build-* dist .deps

format:
	find core modules -type f \( -name '*.c' -o -name '*.h' -o -name '*.cpp' -o -name '*.hpp' \) \
		-print0 | xargs -0 clang-format -i --style=file
