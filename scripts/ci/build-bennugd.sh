#!/usr/bin/env bash
# Configure and build bgdc/bgdi against a static-deps prefix.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
DEPS_DIR="${DEPS_DIR:-$ROOT/.deps}"
PREFIX="${PREFIX:-$DEPS_DIR/prefix}"
BUILD_DIR="${BUILD_DIR:-$ROOT/build-ci}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
STATIC_MODULES="${STATIC_MODULES:-ON}"
JOBS="${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)}"
CMAKE_GENERATOR="${CMAKE_GENERATOR:-}"

is_msys2() { [[ -n "${MSYSTEM:-}" ]]; }
is_msvc_windows() {
  [[ "${RUNNER_OS:-}" == "Windows" ]] && ! is_msys2 && command -v cl >/dev/null 2>&1
}

if [[ ! -d "$PREFIX" ]]; then
  echo "Missing deps prefix: $PREFIX" >&2
  echo "Run scripts/ci/build-static-deps.sh first." >&2
  exit 1
fi

# Resolve static zlib/libpng names across Unix/MinGW (.a) and MSVC (.lib).
PNG_LIB=""
ZLIB_LIB=""
png_candidates=(
  "$PREFIX/lib/libpng16.a" "$PREFIX/lib/libpng.a"
  "$PREFIX/lib/libpng16_static.lib" "$PREFIX/lib/libpng16.lib" "$PREFIX/lib/png16.lib"
)
zlib_candidates=(
  "$PREFIX/lib/libz.a"
  "$PREFIX/lib/libzlibstatic.a"
  "$PREFIX/lib/zlibstatic.lib" "$PREFIX/lib/zlib.lib" "$PREFIX/lib/z.lib"
)
for candidate in "${png_candidates[@]}"; do
  if [[ -f "$candidate" ]]; then PNG_LIB="$candidate"; break; fi
done
if [[ -z "$PNG_LIB" ]]; then
  while IFS= read -r candidate; do
    PNG_LIB="$candidate"
    break
  done < <(find "$PREFIX" \( -name 'libpng16.a' -o -name 'libpng.a' -o -name 'libpng16_static.lib' -o -name 'libpng16.lib' \) 2>/dev/null | head -1)
fi
for candidate in "${zlib_candidates[@]}"; do
  if [[ -f "$candidate" ]]; then ZLIB_LIB="$candidate"; break; fi
done
if [[ -z "$PNG_LIB" || -z "$ZLIB_LIB" ]]; then
  echo "Could not find static PNG/ZLIB under $PREFIX/lib" >&2
  ls -la "$PREFIX/lib" >&2 || true
  find "$PREFIX" \( -iname '*png*.lib' -o -iname '*png*.a' -o -iname '*zlib*.lib' -o -iname 'libz.*' \) 2>/dev/null | head -50 >&2 || true
  exit 1
fi
echo "    PNG_LIBRARY=$PNG_LIB"
echo "    ZLIB_LIBRARY=$ZLIB_LIB"
if is_msys2; then
  echo "    toolchain: MSYS2/${MSYSTEM}"
elif is_msvc_windows; then
  echo "    toolchain: MSVC"
fi

args=(
  -S "$ROOT"
  -B "$BUILD_DIR"
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
  -DCMAKE_PREFIX_PATH="$PREFIX"
  -DBENNUGD_STATIC_DEPS=ON
  -DUSE_LIBDES=ON
  -DSTATIC_MODULES="$STATIC_MODULES"
  -DNO_SOUND=OFF
  -DPNG_LIBRARY="$PNG_LIB"
  -DPNG_PNG_INCLUDE_DIR="$PREFIX/include"
  -DZLIB_LIBRARY="$ZLIB_LIB"
  -DZLIB_INCLUDE_DIR="$PREFIX/include"
)
if [[ -n "$CMAKE_GENERATOR" ]]; then
  args+=(-G "$CMAKE_GENERATOR")
fi
if [[ -n "${CMAKE_GENERATOR_PLATFORM:-}" ]]; then
  args+=(-A "$CMAKE_GENERATOR_PLATFORM")
fi
if is_msvc_windows; then
  args+=(-DCMAKE_POLICY_DEFAULT_CMP0091=NEW)
  args+=(-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded)
fi
# MinGW: avoid depending on libgcc_s_seh-1.dll for redistributable archives.
if is_msys2; then
  args+=(-DCMAKE_EXE_LINKER_FLAGS="-static-libgcc")
  args+=(-DCMAKE_SHARED_LINKER_FLAGS="-static-libgcc")
fi

cmake "${args[@]}"
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" -j"$JOBS"

echo "==> Build complete in $BUILD_DIR (STATIC_MODULES=$STATIC_MODULES)"
