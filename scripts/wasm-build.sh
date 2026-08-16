#!/usr/bin/env bash
# Native bgdc (preset wasm-host) + Emscripten bgdi. Requires emcmake on PATH.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
if [[ -f "${ROOT}/versions.env" ]]; then
  set -a
  # shellcheck disable=SC1091
  source "${ROOT}/versions.env"
  set +a
fi

SRC_DIR="${SRC_DIR:-${ROOT}}"
HOST_BUILD="${ROOT}/build-host"
WASM_BUILD="${ROOT}/build-wasm"
STAGE="${STAGE:-${ROOT}/dist/web-wasm32-static}"
FETCH_DIR="${FETCHCONTENT_BASE_DIR:-${HOST_BUILD}/_deps}"

EXTRA=(
  -DBENNUGD_ZLIB_VERSION="${BENNUGD_ZLIB_VERSION:-${ZLIB_VERSION:-1.3.1}}"
  -DBENNUGD_LIBPNG_VERSION="${BENNUGD_LIBPNG_VERSION:-${LIBPNG_VERSION:-1.6.47}}"
  -DBENNUGD_SDL3_REF="${BENNUGD_SDL3_REF:-${SDL3_REF:-release-3.4.14}}"
  -DBENNUGD_SDL3_MIXER_REF="${BENNUGD_SDL3_MIXER_REF:-${SDL3_MIXER_REF:-release-3.2.4}}"
  -DFETCHCONTENT_BASE_DIR="${FETCH_DIR}"
)
if [[ -n "${BENNUGD_VERSION:-}" ]]; then
  EXTRA+=(-DBENNUGD_VERSION="${BENNUGD_VERSION}")
fi

emcc -v
# emsdk / setup-emsdk export CC=emcc; the host bgdc must use a native compiler.
unset CC CXX CFLAGS CXXFLAGS
cmake --preset wasm-host "${EXTRA[@]}"
cmake --build --preset wasm-host

for prg in "${SRC_DIR}"/web/demo/*.prg; do
  dcb="${prg%.prg}.dcb"
  "${HOST_BUILD}/core/bgdc/src/bgdc" -o "${dcb}" "${prg}"
  test -s "${dcb}"
done
ls -l "${SRC_DIR}"/web/demo/*.dcb

emcmake cmake -S "${SRC_DIR}" -B "${WASM_BUILD}" -G Ninja \
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE:-Release}" \
  -DUSE_LIBDES=ON \
  -DBENNUGD_BUNDLE_DEPS=ON \
  -DSTATIC_MODULES=ON \
  -DINTERPRETER_ONLY=ON \
  "${EXTRA[@]}" \
  -DFETCHCONTENT_SOURCE_DIR_ZLIB="${FETCH_DIR}/zlib-src" \
  -DFETCHCONTENT_SOURCE_DIR_LIBPNG="${FETCH_DIR}/libpng-src" \
  -DFETCHCONTENT_SOURCE_DIR_SDL3="${FETCH_DIR}/sdl3-src" \
  -DFETCHCONTENT_SOURCE_DIR_SDL3_MIXER="${FETCH_DIR}/sdl3_mixer-src"

cmake --build "${WASM_BUILD}" --target bgdi

SRC="${WASM_BUILD}/core/bgdi/src"
mkdir -p "${STAGE}"
cp "${SRC}/bgdi.html" "${STAGE}/index.html"
cp "${SRC}/bgdi.html" "${SRC}/bgdi.js" "${SRC}/bgdi.wasm" "${SRC}/bgdi.data" "${STAGE}/"
cp "${SRC_DIR}/README.md" "${STAGE}/"
cp "${WASM_BUILD}/BUILD_INFO.txt" "${STAGE}/"
cp "${SRC}/bgdi.wasm.map" "${STAGE}/" 2>/dev/null || true
cp "${SRC}/bgdi.js.map" "${STAGE}/" 2>/dev/null || true

ls -lah "${STAGE}"
test -s "${STAGE}/index.html"
test -s "${STAGE}/bgdi.html"
test -s "${STAGE}/bgdi.js"
test -s "${STAGE}/bgdi.wasm"
test -s "${STAGE}/bgdi.data"
