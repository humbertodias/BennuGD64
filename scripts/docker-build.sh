#!/usr/bin/env bash
# Build artifacts in a toolchain image (CMake presets + CTest).
#
#   bash scripts/docker-build.sh
#   bash scripts/docker-build.sh linux shared
#   bash scripts/docker-build.sh windows
#   bash scripts/docker-build.sh wasm
#   bash scripts/docker-build.sh linux shell
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "${ROOT}"

USAGE="usage: $0 linux|windows [static|shared|shell]
       $0 wasm [shell]"

if [[ -f "${ROOT}/versions.env" ]]; then
  set -a
  # shellcheck disable=SC1091
  source "${ROOT}/versions.env"
  set +a
fi

if ! command -v docker >/dev/null 2>&1; then
  echo "Docker is required (https://docs.docker.com/get-docker/)." >&2
  exit 1
fi

PLATFORM="${1:-linux}"
SECOND="${2:-}"
if [[ "${PLATFORM}" == *-* && -z "${SECOND}" ]]; then
  SECOND="${PLATFORM#*-}"
  PLATFORM="${PLATFORM%%-*}"
fi
SECOND="${SECOND:-static}"
case "${PLATFORM}" in
  linux|windows|wasm) ;;
  *)
    echo "${USAGE}" >&2
    exit 1
    ;;
esac

IMAGE="bennugd64-${PLATFORM}"

if [[ "${SKIP_DOCKER_BUILD:-}" != "1" ]]; then
  if [[ "${PLATFORM}" == "wasm" ]]; then
    docker build \
      --target wasm \
      --build-arg EMSCRIPTEN_VERSION="${EMSCRIPTEN_VERSION:-6.0.6}" \
      -t bennugd64-wasm \
      -f docker/Dockerfile.linux \
      docker/
  elif [[ "${PLATFORM}" == "linux" ]]; then
    docker build --target linux -t bennugd64-linux -f docker/Dockerfile.linux docker/
  else
    docker build -t "${IMAGE}" -f "docker/Dockerfile.${PLATFORM}" docker/
  fi
fi

if [[ "${SECOND}" == "shell" ]]; then
  exec docker run --rm -it \
    -v "${ROOT}:/src" \
    -w /src \
    -e HOME=/tmp \
    "${IMAGE}" bash
fi

if [[ -z "${BENNUGD_VERSION:-}" ]]; then
  BENNUGD_VERSION="$(git describe --tags --exact-match HEAD 2>/dev/null \
    || git describe --tags --always --dirty 2>/dev/null \
    || echo dev)"
fi

# FetchContent *-subbuild embeds the configure-time absolute path. A cache
# from the GitHub runner or a host configure will not work at /src in Docker.
scrub_fetchcontent() {
  local d="$1"
  [[ -d "${d}" ]] || return 0
  find "${d}" -mindepth 1 -maxdepth 1 -type d \
    \( -name '*-subbuild' -o -name '*-build' -o -name '*-tmp' \) \
    -exec rm -rf {} +
}

if [[ "${PLATFORM}" == "wasm" ]]; then
  echo "image: ${IMAGE}"
  echo "preset: wasm-host + emcmake"
  echo "version: ${BENNUGD_VERSION}"
  scrub_fetchcontent "${ROOT}/build-host/_deps"
  docker run --rm \
    -u "$(id -u):$(id -g)" \
    -v "${ROOT}:/src" \
    -w /src \
    -e HOME=/tmp \
    -e BENNUGD_VERSION="${BENNUGD_VERSION}" \
    -e BUILD_TYPE="${BUILD_TYPE:-Release}" \
    -e ZLIB_VERSION="${ZLIB_VERSION:-1.3.1}" \
    -e LIBPNG_VERSION="${LIBPNG_VERSION:-1.6.47}" \
    -e SDL3_REF="${SDL3_REF:-release-3.4.14}" \
    -e SDL3_MIXER_REF="${SDL3_MIXER_REF:-release-3.2.4}" \
    "${IMAGE}" \
    bash -c 'set -euo pipefail
      set +eu
      # shellcheck disable=SC1091
      source "${EMSDK}/emsdk_env.sh"
      set -euo pipefail
      unset CC CXX CFLAGS CXXFLAGS
      HOST_BUILD=/src/build-host
      WASM_BUILD=/src/build-wasm
      STAGE=/src/dist/web-wasm32-static
      FETCH_DIR="${HOST_BUILD}/_deps"
      EXTRA=(
        -DBENNUGD_VERSION="${BENNUGD_VERSION}"
        -DBENNUGD_ZLIB_VERSION="${ZLIB_VERSION}"
        -DBENNUGD_LIBPNG_VERSION="${LIBPNG_VERSION}"
        -DBENNUGD_SDL3_REF="${SDL3_REF}"
        -DBENNUGD_SDL3_MIXER_REF="${SDL3_MIXER_REF}"
        -DFETCHCONTENT_BASE_DIR="${FETCH_DIR}"
      )
      emcc -v
      cmake --preset wasm-host "${EXTRA[@]}"
      cmake --build --preset wasm-host
      for prg in /src/web/demo/*.prg; do
        dcb="${prg%.prg}.dcb"
        "${HOST_BUILD}/core/bgdc/src/bgdc" -o "${dcb}" "${prg}"
        test -s "${dcb}"
      done
      emcmake cmake -S /src -B "${WASM_BUILD}" -G Ninja \
        -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
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
      cp /src/README.md "${STAGE}/"
      cp "${WASM_BUILD}/BUILD_INFO.txt" "${STAGE}/"
      cp "${SRC}/bgdi.wasm.map" "${STAGE}/" 2>/dev/null || true
      cp "${SRC}/bgdi.js.map" "${STAGE}/" 2>/dev/null || true
      test -s "${STAGE}/index.html"
      test -s "${STAGE}/bgdi.html"
      test -s "${STAGE}/bgdi.js"
      test -s "${STAGE}/bgdi.wasm"
      test -s "${STAGE}/bgdi.data"
    '
  exit 0
fi

LINKAGE="${SECOND}"
case "${LINKAGE}" in
  static|shared) ;;
  *)
    echo "${USAGE}" >&2
    exit 1
    ;;
esac

if [[ "${PLATFORM}" == "windows" ]]; then
  PRESET="windows-${LINKAGE}"
  BUILD_DIR="/src/build-windows-${LINKAGE}"
  STAGE="/src/dist/windows-x86_64-${LINKAGE}"
else
  PRESET="${LINKAGE}"
  BUILD_DIR="/src/build-${LINKAGE}"
  STAGE="/src/dist/linux-${LINKAGE}"
fi

echo "image: ${IMAGE}"
echo "preset: ${PRESET}"
echo "version: ${BENNUGD_VERSION}"

scrub_fetchcontent "${ROOT}/${BUILD_DIR#/src/}/_deps"

docker run --rm \
  -u "$(id -u):$(id -g)" \
  -v "${ROOT}:/src" \
  -w /src \
  -e HOME=/tmp \
  -e BENNUGD_VERSION="${BENNUGD_VERSION}" \
  -e ZLIB_VERSION="${ZLIB_VERSION:-1.3.1}" \
  -e LIBPNG_VERSION="${LIBPNG_VERSION:-1.6.47}" \
  -e SDL3_REF="${SDL3_REF:-release-3.4.14}" \
  -e SDL3_MIXER_REF="${SDL3_MIXER_REF:-release-3.2.4}" \
  -e PRESET="${PRESET}" \
  -e BUILD_DIR="${BUILD_DIR}" \
  -e STAGE="${STAGE}" \
  "${IMAGE}" \
  bash -c 'set -euo pipefail
    cmake --preset "${PRESET}" \
      -DBENNUGD_VERSION="${BENNUGD_VERSION}" \
      -DBENNUGD_ZLIB_VERSION="${ZLIB_VERSION}" \
      -DBENNUGD_LIBPNG_VERSION="${LIBPNG_VERSION}" \
      -DBENNUGD_SDL3_REF="${SDL3_REF}" \
      -DBENNUGD_SDL3_MIXER_REF="${SDL3_MIXER_REF}"
    cmake --build --preset "${PRESET}"
    cmake --install "${BUILD_DIR}" --prefix "${STAGE}"
    if [[ "${PRESET}" != windows-* ]]; then
      ctest --preset "${PRESET}" --output-on-failure
    fi
  '
