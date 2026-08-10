#!/usr/bin/env bash
# Build zlib, libpng, SDL3 and SDL3_mixer as static libraries into a prefix.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
DEPS_DIR="${DEPS_DIR:-$ROOT/.deps}"
PREFIX="${PREFIX:-$DEPS_DIR/prefix}"
SRC_DIR="${SRC_DIR:-$DEPS_DIR/src}"
JOBS="${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)}"

ZLIB_VERSION="${ZLIB_VERSION:-1.3.1}"
LIBPNG_VERSION="${LIBPNG_VERSION:-1.6.47}"
# SDL3_mixer 3.2.x needs SDL 3.4+ APIs (e.g. SDL_PutAudioStreamDataNoCopy).
SDL3_REF="${SDL3_REF:-release-3.4.14}"
SDL3_MIXER_REF="${SDL3_MIXER_REF:-release-3.2.4}"

BUILD_TYPE="${BUILD_TYPE:-Release}"
CMAKE_GENERATOR="${CMAKE_GENERATOR:-}"
HOST_OS="$(uname -s)"

mkdir -p "$SRC_DIR" "$PREFIX"
# Avoid bash failing on unmatched globs in cleanup paths (esp. Windows).
shopt -s nullglob

cmake_configure() {
  local src="$1"
  local build="$2"
  shift 2
  local args=(
    -S "$src"
    -B "$build"
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    -DCMAKE_INSTALL_PREFIX="$PREFIX"
    -DCMAKE_PREFIX_PATH="$PREFIX"
    -DCMAKE_FIND_PACKAGE_PREFER_CONFIG=ON
  )
  if [[ -n "$CMAKE_GENERATOR" ]]; then
    args+=(-G "$CMAKE_GENERATOR")
  fi
  if [[ -n "${CMAKE_GENERATOR_PLATFORM:-}" ]]; then
    args+=(-A "$CMAKE_GENERATOR_PLATFORM")
  fi
  if [[ "${RUNNER_OS:-}" == "Windows" || "$HOST_OS" == MINGW* || "$HOST_OS" == MSYS* ]]; then
    args+=(-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded)
    # MSVC is not a usable ASM compiler for vendored deps (CMP0194).
    args+=(-DCMAKE_POLICY_DEFAULT_CMP0194=NEW)
  fi
  cmake "${args[@]}" "$@"
}

cmake_build_install() {
  local build="$1"
  echo "    building $build"
  cmake --build "$build" --config "$BUILD_TYPE" -j"$JOBS"
  echo "    installing $build -> $PREFIX"
  cmake --install "$build" --config "$BUILD_TYPE"
}

fetch_tarball() {
  local dest="$1"
  shift
  if [[ -d "$dest" && -n "$(ls -A "$dest" 2>/dev/null || true)" ]]; then
    return 0
  fi
  local tmp url
  tmp="$(mktemp -d)"
  mkdir -p "$dest"
  for url in "$@"; do
    echo "Downloading $url"
    if curl -fsSL "$url" -o "$tmp/archive.tgz"; then
      tar -xzf "$tmp/archive.tgz" -C "$tmp"
      local extracted
      extracted="$(find "$tmp" -mindepth 1 -maxdepth 1 -type d | head -1)"
      shopt -s dotglob nullglob
      rm -rf "${dest:?}/"*
      mv "$extracted"/* "$dest"/
      shopt -u dotglob nullglob
      rm -rf "$tmp"
      return 0
    fi
    echo "Failed: $url" >&2
  done
  rm -rf "$tmp"
  echo "Could not download archive for $dest" >&2
  exit 1
}

fetch_git() {
  local url="$1"
  local ref="$2"
  local dest="$3"
  local with_submodules="${4:-0}"
  if [[ -d "$dest/.git" ]]; then
    git -C "$dest" fetch --depth 1 origin "$ref" || true
    git -C "$dest" fetch --depth 1 origin "refs/tags/${ref}:refs/tags/${ref}" || true
    git -C "$dest" checkout -f "$ref"
  else
    rm -rf "$dest"
    if ! git clone --depth 1 --branch "$ref" "$url" "$dest"; then
      git clone --depth 1 "$url" "$dest"
      git -C "$dest" fetch --depth 1 origin "refs/tags/${ref}:refs/tags/${ref}" || \
        git -C "$dest" fetch --depth 1 origin "$ref"
      git -C "$dest" checkout -f "$ref"
    fi
  fi
  if [[ "$with_submodules" == "1" ]]; then
    # Vendored codecs (ogg/vorbis/etc.) live in git submodules.
    git -C "$dest" submodule sync --recursive
    git -C "$dest" submodule update --init --recursive --depth 1
    # Fallback: SDL_mixer ships a helper that populates external/ when needed.
    if [[ -x "$dest/external/download.sh" && ! -f "$dest/external/ogg/CMakeLists.txt" ]]; then
      (cd "$dest/external" && ./download.sh)
    fi
  fi
}

echo "==> Installing static deps into $PREFIX"

# --- zlib ---
echo "==> [1/4] zlib ${ZLIB_VERSION}"
fetch_tarball "$SRC_DIR/zlib" \
  "https://github.com/madler/zlib/releases/download/v${ZLIB_VERSION}/zlib-${ZLIB_VERSION}.tar.gz"
cmake_configure "$SRC_DIR/zlib" "$SRC_DIR/zlib-build" \
  -DBUILD_SHARED_LIBS=OFF \
  -DZLIB_BUILD_EXAMPLES=OFF
cmake_build_install "$SRC_DIR/zlib-build"
rm -f "$PREFIX/lib/libz.dylib" "$PREFIX/lib/libz."*.dylib
# MSVC installs zlibstatic.lib; give FindZLIB a conventional zlib.lib alias.
if [[ -f "$PREFIX/lib/zlibstatic.lib" && ! -f "$PREFIX/lib/zlib.lib" ]]; then
  cp "$PREFIX/lib/zlibstatic.lib" "$PREFIX/lib/zlib.lib"
fi
ZLIB_LIB=""
for candidate in "$PREFIX/lib/zlibstatic.lib" "$PREFIX/lib/zlib.lib" "$PREFIX/lib/libz.a" "$PREFIX/lib/z.lib"; do
  if [[ -f "$candidate" ]]; then ZLIB_LIB="$candidate"; break; fi
done
if [[ -z "$ZLIB_LIB" ]]; then
  echo "zlib static library missing under $PREFIX/lib" >&2
  ls -la "$PREFIX/lib" >&2 || true
  exit 1
fi
echo "    using ZLIB_LIB=$ZLIB_LIB"

# --- libpng ---
echo "==> [2/4] libpng ${LIBPNG_VERSION}"
fetch_tarball "$SRC_DIR/libpng" \
  "https://github.com/pnggroup/libpng/archive/refs/tags/v${LIBPNG_VERSION}.tar.gz" \
  "https://downloads.sourceforge.net/project/libpng/libpng16/${LIBPNG_VERSION}/libpng-${LIBPNG_VERSION}.tar.gz"
cmake_configure "$SRC_DIR/libpng" "$SRC_DIR/libpng-build" \
  -DBUILD_SHARED_LIBS=OFF \
  -DPNG_SHARED=OFF \
  -DPNG_STATIC=ON \
  -DPNG_FRAMEWORK=OFF \
  -DPNG_TESTS=OFF \
  -DPNG_TOOLS=OFF \
  -DZLIB_ROOT="$PREFIX" \
  -DZLIB_INCLUDE_DIR="$PREFIX/include" \
  -DZLIB_LIBRARY="$ZLIB_LIB"
cmake_build_install "$SRC_DIR/libpng-build"
# Prefer the static archive over any accidental framework install on macOS.
rm -rf "$PREFIX/lib/png.framework" "$PREFIX/lib/libpng.dylib" "$PREFIX/lib/libpng16.dylib"
if [[ -f "$PREFIX/lib/libpng16_static.lib" && ! -f "$PREFIX/lib/libpng16.lib" ]]; then
  cp "$PREFIX/lib/libpng16_static.lib" "$PREFIX/lib/libpng16.lib"
fi

# --- SDL3 ---
echo "==> [3/4] SDL3 ${SDL3_REF}"
# Drop stale installs when upgrading major/minor so linker never mixes 3.2 with mixer 3.2.4+.
rm -rf "$SRC_DIR/SDL-build" \
  "$PREFIX/lib/cmake/SDL3" \
  "$PREFIX/cmake/SDL3" \
  "$PREFIX/include/SDL3" \
  "$PREFIX/lib/pkgconfig/sdl3.pc"
rm -f "$PREFIX/lib"/libSDL3* "$PREFIX/lib"/SDL3* "$PREFIX/lib"/sdl3*
fetch_git "https://github.com/libsdl-org/SDL.git" "$SDL3_REF" "$SRC_DIR/SDL"
cmake_configure "$SRC_DIR/SDL" "$SRC_DIR/SDL-build" \
  -DSDL_SHARED=OFF \
  -DSDL_STATIC=ON \
  -DSDL_TEST_LIBRARY=OFF \
  -DSDL_TESTS=OFF \
  -DSDL_INSTALL_DOCS=OFF \
  -DSDL_LIBUSB=OFF \
  -DSDL_HIDAPI_LIBUSB=OFF
cmake_build_install "$SRC_DIR/SDL-build"

# --- SDL3_mixer ---
# Use header-only decoders (stb_vorbis + dr_mp3) so we do not need git submodules
# or vendored flac/mpg123/opus (those break easily under MSVC).
# NOTE: the CMake option is SDLMIXER_EXAMPLES, not SDLMIXER_SAMPLES.
echo "==> [4/4] SDL3_mixer ${SDL3_MIXER_REF}"
rm -rf "$SRC_DIR/SDL_mixer-build" \
  "$PREFIX/lib/cmake/SDL3_mixer" \
  "$PREFIX/cmake/SDL3_mixer" \
  "$PREFIX/lib/pkgconfig/sdl3-mixer.pc" \
  "$PREFIX/lib/pkgconfig/sdl3_mixer.pc"
rm -f "$PREFIX/lib"/libSDL3_mixer* "$PREFIX/lib"/SDL3_mixer*
fetch_git "https://github.com/libsdl-org/SDL_mixer.git" "$SDL3_MIXER_REF" "$SRC_DIR/SDL_mixer" 0
SDL3_DIR_HINT=""
for d in "$PREFIX/lib/cmake/SDL3" "$PREFIX/cmake/SDL3"; do
  if [[ -f "$d/SDL3Config.cmake" ]]; then SDL3_DIR_HINT="$d"; break; fi
done
if [[ -z "$SDL3_DIR_HINT" ]]; then
  echo "SDL3Config.cmake not found after SDL3 install" >&2
  find "$PREFIX" -name 'SDL3Config.cmake' 2>/dev/null | head >&2 || true
  exit 1
fi
cmake_configure "$SRC_DIR/SDL_mixer" "$SRC_DIR/SDL_mixer-build" \
  -DSDL3_DIR="$SDL3_DIR_HINT" \
  -DBUILD_SHARED_LIBS=OFF \
  -DSDLMIXER_DEPS_SHARED=OFF \
  -DSDLMIXER_VENDORED=OFF \
  -DSDLMIXER_EXAMPLES=OFF \
  -DSDLMIXER_TESTS=OFF \
  -DSDLMIXER_STRICT=OFF \
  -DSDLMIXER_GME=OFF \
  -DSDLMIXER_MOD=OFF \
  -DSDLMIXER_MIDI=OFF \
  -DSDLMIXER_WAVPACK=OFF \
  -DSDLMIXER_FLAC=OFF \
  -DSDLMIXER_OPUS=OFF \
  -DSDLMIXER_MP3=ON \
  -DSDLMIXER_MP3_DRMP3=ON \
  -DSDLMIXER_MP3_MPG123=OFF \
  -DSDLMIXER_VORBIS_STB=ON \
  -DSDLMIXER_VORBIS_VORBISFILE=OFF \
  -DSDLMIXER_VORBIS_TREMOR=OFF
cmake_build_install "$SRC_DIR/SDL_mixer-build"

# Sanity: mixer 3.2.4+ requires SDL 3.4+.
SDL_VER=""
for vf in \
  "$PREFIX/lib/cmake/SDL3/SDL3ConfigVersion.cmake" \
  "$PREFIX/cmake/SDL3/SDL3ConfigVersion.cmake"
do
  if [[ -f "$vf" ]]; then
    SDL_VER="$(sed -n 's/set(PACKAGE_VERSION "\([^"]*\)")/\1/p' "$vf" | head -1)"
    break
  fi
done
if [[ -z "$SDL_VER" ]]; then
  echo "Could not determine installed SDL3 version under $PREFIX" >&2
  find "$PREFIX" -name 'SDL3ConfigVersion.cmake' 2>/dev/null | head >&2 || true
  exit 1
fi
if [[ "$(printf '%s\n' "3.4.0" "$SDL_VER" | sort -V | head -1)" != "3.4.0" ]]; then
  echo "Installed SDL3 $SDL_VER is too old for SDL3_mixer (need >= 3.4.0, SDL3_REF=$SDL3_REF)." >&2
  exit 1
fi
echo "==> SDL3 $SDL_VER OK"

echo "==> Static deps ready at $PREFIX"
ls -la "$PREFIX/lib" 2>/dev/null || ls -la "$PREFIX/lib64" 2>/dev/null || true
