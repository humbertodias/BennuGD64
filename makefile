BUILD_TYPE := Debug
CMAKE_FLAGS := -DUSE_LIBDES=ON
PREFIX_STATIC ?= dist/bennugd64
PREFIX_SHARED ?= dist/bennugd64-shared

WASI_SDK_VERSION ?= 33
WASI_SDK_VERSION_FULL ?= 33.0
ZLIB_VERSION ?= 1.3.1

.PHONY: all static shared install/static install/shared install/wasmtime wasm clean format

all: static

# Modules linked into bgdi
static:
	cmake -S . -B build-static -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) $(CMAKE_FLAGS) -DSTATIC_MODULES=ON
	cmake --build build-static

# Modules as .so/.dll/.dylib under modules/
shared:
	cmake -S . -B build-shared -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) $(CMAKE_FLAGS) -DSTATIC_MODULES=OFF
	cmake --build build-shared

install/static: static
	cmake --install build-static --prefix $(PREFIX_STATIC)

install/shared: shared
	cmake --install build-shared --prefix $(PREFIX_SHARED)

# Browser interpreter (needs emsdk: source emsdk_env.sh, then this target)
wasm/build:
	@bgdc=build-static/core/bgdc/src/bgdc; \
	if [ ! -x "$$bgdc" ]; then echo "Build native bgdc first (make static)"; exit 1; fi; \
	for prg in web/demo/*.prg; do \
		dcb="$${prg%.prg}.dcb"; \
		"$$bgdc" -o "$$dcb" "$$prg"; \
	done
	emcmake cmake -S . -B build-wasm -DCMAKE_BUILD_TYPE=Release $(CMAKE_FLAGS) \
		-DSTATIC_MODULES=ON -DINTERPRETER_ONLY=ON
	cmake --build build-wasm

wasi/build:
	cmake -S . -B build-wasi -G Ninja \
	            -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
	            -DBENNUGD_WASI=ON \
	            -DBENNUGD_WASI_SDK_VERSION="${WASI_SDK_VERSION}" \
	            -DBENNUGD_WASI_SDK_VERSION_FULL="${WASI_SDK_VERSION_FULL}" \
	            -DUSE_LIBDES=ON \
	            -DCOMPILER_ONLY=ON \
	            -DSTATIC_MODULES=ON \
	            -DBENNUGD_BUNDLE_DEPS=ON \
	            -DBENNUGD_ZLIB_VERSION="${ZLIB_VERSION}"
	cmake --build build-wasi --target bgdc

wasi/run:
	wasmtime --dir=. "build-wasi/core/bgdc/src/bgdc.wasm" web/demo/hello.prg

install/wasmtime:
	curl https://wasmtime.dev/install.sh -sSf | bash

wasm/server:
	python3 -m http.server 8080 --directory build-wasm/core/bgdi/src

clean:
	rm -rf build-static build-shared build-wasm build-wasi

format:
	find core modules -type f \( -name '*.c' -o -name '*.h' -o -name '*.cpp' -o -name '*.hpp' \) \
		-print0 | xargs -0 clang-format -i --style=file
