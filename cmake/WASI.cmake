# wasm32-wasi: compiler only (bgdc.wasm). No SDL, no interpreter.
# Configure with:
#   cmake -B build-wasi -DBENNUGD_WASI=ON
#   cmake --build build-wasi --target bgdc
# Or pass an existing SDK:
#   cmake -B build-wasi -DCMAKE_TOOLCHAIN_FILE=$WASI_SDK_PATH/share/cmake/wasi-sdk-p1.cmake
# Run with:
#   wasmtime --dir=. ./bgdc.wasm -- -o out.dcb in.prg

set (STATIC_MODULES ON CACHE BOOL "WASI cannot load shared Bennu modules" FORCE)
set (COMPILER_ONLY ON CACHE BOOL "WASI builds the compiler, not the SDL interpreter" FORCE)
set (USE_LIBDES ON CACHE BOOL "OpenSSL is not used on WASI" FORCE)
set (NO_SOUND ON CACHE BOOL "No audio on the WASI compiler" FORCE)

if (NOT CMAKE_EXECUTABLE_SUFFIX)
  set (CMAKE_EXECUTABLE_SUFFIX ".wasm")
endif ()
set (CMAKE_EXECUTABLE_SUFFIX_C ".wasm")

add_compile_definitions (TARGET_WASI)

# Compiler recursion and token buffers need more than the WASI default stack.
add_link_options (
  -Wl,--stack-first
  -Wl,-z,stack-size=1048576
)
