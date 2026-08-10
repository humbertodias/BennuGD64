#!/usr/bin/env bash
# Configure and build bgdc/bgdi against a static-deps prefix.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
DEPS_DIR="${DEPS_DIR:-$ROOT/.deps}"
PREFIX="${PREFIX:-$DEPS_DIR/prefix}"
BUILD_DIR="${BUILD_DIR:-$ROOT/build-ci}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
JOBS="${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)}"
CMAKE_GENERATOR="${CMAKE_GENERATOR:-}"

if [[ ! -d "$PREFIX" ]]; then
  echo "Missing deps prefix: $PREFIX" >&2
  echo "Run scripts/ci/build-static-deps.sh first." >&2
  exit 1
fi

args=(
  -S "$ROOT"
  -B "$BUILD_DIR"
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
  -DCMAKE_PREFIX_PATH="$PREFIX"
  -DBENNUGD_STATIC_DEPS=ON
  -DUSE_LIBDES=ON
  -DSTATIC_MODULES=ON
  -DNO_SOUND=OFF
  -DPNG_LIBRARY="$PREFIX/lib/libpng16.a"
  -DPNG_PNG_INCLUDE_DIR="$PREFIX/include"
  -DZLIB_LIBRARY="$PREFIX/lib/libz.a"
  -DZLIB_INCLUDE_DIR="$PREFIX/include"
)
if [[ -n "$CMAKE_GENERATOR" ]]; then
  args+=(-G "$CMAKE_GENERATOR")
fi
if [[ -n "${CMAKE_GENERATOR_PLATFORM:-}" ]]; then
  args+=(-A "$CMAKE_GENERATOR_PLATFORM")
fi
if [[ "${RUNNER_OS:-}" == "Windows" || "$(uname -s)" == MINGW* || "$(uname -s)" == MSYS* ]]; then
  args+=(-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded)
fi

cmake "${args[@]}"
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" -j"$JOBS"

echo "==> Build complete in $BUILD_DIR"
