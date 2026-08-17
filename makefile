include versions.env
export

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
PREFIX_STATIC ?= dist/macos-arm64-static
PREFIX_SHARED ?= dist/macos-arm64-shared
else
PREFIX_STATIC ?= dist/linux-static
PREFIX_SHARED ?= dist/linux-shared
endif

.PHONY: all static shared wasm wasi docker-linux docker-linux-shared \
	docker-windows docker-windows-shared docker-android \
	wasi/run wasm/server install/wasmtime clean format

all: static

static:
	cmake --preset static
	cmake --build --preset static
	cmake --install build-static --prefix $(PREFIX_STATIC)
	ctest --preset static --output-on-failure

shared:
	cmake --preset shared
	cmake --build --preset shared
	cmake --install build-shared --prefix $(PREFIX_SHARED)
	ctest --preset shared --output-on-failure

wasm:
	bash scripts/docker-build.sh wasm

wasi:
	cmake --preset wasi
	cmake --build --preset wasi
	cmake --install build-wasi --prefix dist/wasi-wasm32-static
	ctest --preset wasi --output-on-failure

docker-linux:
	bash scripts/docker-build.sh linux

docker-linux-shared:
	bash scripts/docker-build.sh linux shared

docker-windows:
	bash scripts/docker-build.sh windows

docker-windows-shared:
	bash scripts/docker-build.sh windows shared

docker-android:
	bash scripts/docker-build.sh android

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
