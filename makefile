BUILD_TYPE := Debug
CMAKE_FLAGS := -DUSE_LIBDES=ON
PREFIX_STATIC ?= dist/bennugd64
PREFIX_SHARED ?= dist/bennugd64-shared

.PHONY: all static shared install-static install-shared clean format

all: static

# Modules linked into bgdi
static:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) $(CMAKE_FLAGS) -DSTATIC_MODULES=ON
	cmake --build build

# Modules as .so/.dll/.dylib under modules/
shared:
	cmake -S . -B build-shared -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) $(CMAKE_FLAGS) -DSTATIC_MODULES=OFF
	cmake --build build-shared

install-static: static
	cmake --install build --prefix $(PREFIX_STATIC)

install-shared: shared
	cmake --install build-shared --prefix $(PREFIX_SHARED)

build/bgdc:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) $(CMAKE_FLAGS) -DSTATIC_MODULES=ON
	cmake --build build --target bgdc

build/bgdi:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) $(CMAKE_FLAGS) -DSTATIC_MODULES=ON
	cmake --build build --target bgdi

clean:
	rm -rf build build-shared

format:
	find core modules -type f \( -name '*.c' -o -name '*.h' -o -name '*.cpp' -o -name '*.hpp' \) \
		-print0 | xargs -0 clang-format -i --style=file
