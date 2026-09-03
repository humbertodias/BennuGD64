#!/usr/bin/env bash
# Build artifacts in a toolchain image (CMake presets + CTest).
#
#   bash scripts/build.sh
#   bash scripts/build.sh linux shared
#   bash scripts/build.sh windows
#   bash scripts/build.sh wasm
#   bash scripts/build.sh android
#   bash scripts/build.sh switch
#   bash scripts/build.sh dreamcast
#   bash scripts/build.sh psp
#   bash scripts/build.sh vita
#   bash scripts/build.sh tvos
#   bash scripts/build.sh tvos simulator
#   bash scripts/build.sh ios
#   bash scripts/build.sh ios simulator
#   bash scripts/build.sh ps2
#   bash scripts/build.sh ps3
#   bash scripts/build.sh pandora
#   bash scripts/build.sh wii
#   bash scripts/build.sh macos
#   bash scripts/build.sh macos arm64
#   bash scripts/build.sh macos arm64 shared
#   bash scripts/build.sh linux shell
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "${ROOT}"

USAGE="usage: $0 linux|windows [static|shared|shell]
       $0 wasm [shell]
       $0 android [shell]
       $0 switch [shell]
       $0 dreamcast [shell]
       $0 psp [shell]
       $0 vita [shell]
       $0 tvos [device|simulator|shell]
       $0 ios [device|simulator|shell]
       $0 ps2 [shell]
       $0 ps3 [shell]
       $0 pandora [shell]
       $0 wii [shell]
       $0 macos [x86_64|arm64] [static|shared|shell]"

if [[ -f "${ROOT}/versions.env" ]]; then
  set -a
  # shellcheck disable=SC1091
  source "${ROOT}/versions.env"
  set +a
fi

PLATFORM="${1:-linux}"
SECOND="${2:-}"
if [[ "${PLATFORM}" == *-* && -z "${SECOND}" ]]; then
  SECOND="${PLATFORM#*-}"
  PLATFORM="${PLATFORM%%-*}"
fi
SECOND="${SECOND:-static}"
case "${PLATFORM}" in
  linux|windows|wasm|android|switch|dreamcast|psp|vita|tvos|ios|ps2|ps3|pandora|wii|macos) ;;
  *)
    echo "${USAGE}" >&2
    exit 1
    ;;
esac

APPLE_SIM=0
if [[ "${PLATFORM}" == "tvos" || "${PLATFORM}" == "ios" ]] && [[ "${SECOND}" == "simulator" ]]; then
  APPLE_SIM=1
fi
NATIVE_APPLE_SIM=0
if [[ "${APPLE_SIM}" == "1" && "$(uname -s)" == "Darwin" ]]; then
  NATIVE_APPLE_SIM=1
fi

if [[ "${NATIVE_APPLE_SIM}" != "1" ]] && ! command -v docker >/dev/null 2>&1; then
  echo "Docker is required (https://docs.docker.com/get-docker/)." >&2
  exit 1
fi

IMAGE="bennugd64-${PLATFORM}"

if [[ "${SKIP_DOCKER_BUILD:-}" != "1" && "${NATIVE_APPLE_SIM}" != "1" ]]; then
  if [[ "${PLATFORM}" == "wasm" ]]; then
    docker build \
      --build-arg EMSCRIPTEN_VERSION="${EMSCRIPTEN_VERSION:-6.0.6}" \
      -t bennugd64-wasm \
      -f docker/Dockerfile.wasm \
      docker/
  elif [[ "${PLATFORM}" == "linux" ]]; then
    docker build --target linux -t bennugd64-linux -f docker/Dockerfile.linux docker/
  elif [[ "${PLATFORM}" == "android" ]]; then
    docker build \
      --platform linux/amd64 \
      --build-arg ANDROID_NDK_VERSION="${ANDROID_NDK_VERSION:-27.0.12077973}" \
      --build-arg ANDROID_CMDLINE_TOOLS="${ANDROID_CMDLINE_TOOLS:-11076708}" \
      --build-arg ANDROID_COMPILE_SDK="${ANDROID_COMPILE_SDK:-34}" \
      --build-arg ANDROID_BUILD_TOOLS="${ANDROID_BUILD_TOOLS:-35.0.0}" \
      --build-arg GRADLE_VERSION="${GRADLE_VERSION:-8.12}" \
      -t bennugd64-android \
      -f docker/Dockerfile.android \
      docker/
  elif [[ "${PLATFORM}" == "switch" ]]; then
    docker build \
      --platform linux/amd64 \
      -t bennugd64-switch \
      -f docker/Dockerfile.switch \
      docker/
  elif [[ "${PLATFORM}" == "dreamcast" ]]; then
    docker build \
      --platform linux/amd64 \
      -t bennugd64-dreamcast \
      -f docker/Dockerfile.dreamcast \
      docker/
  elif [[ "${PLATFORM}" == "psp" ]]; then
    docker build \
      --platform linux/amd64 \
      -t bennugd64-psp \
      -f docker/Dockerfile.psp \
      docker/
  elif [[ "${PLATFORM}" == "vita" ]]; then
    docker build \
      -t bennugd64-vita \
      -f docker/Dockerfile.vita \
      docker/
  elif [[ "${PLATFORM}" == "ps2" ]]; then
    docker build \
      --platform linux/amd64 \
      -t bennugd64-ps2 \
      -f docker/Dockerfile.ps2 \
      docker/
  elif [[ "${PLATFORM}" == "ps3" ]]; then
    docker build \
      --platform linux/amd64 \
      -t bennugd64-ps3 \
      -f docker/Dockerfile.ps3 \
      docker/
  elif [[ "${PLATFORM}" == "pandora" ]]; then
    docker build \
      --platform linux/amd64 \
      -t bennugd64-pandora \
      -f docker/Dockerfile.pandora \
      docker/
  elif [[ "${PLATFORM}" == "wii" ]]; then
    docker build \
      --platform linux/amd64 \
      -t bennugd64-wii \
      -f docker/Dockerfile.wii \
      docker/
  elif [[ "${PLATFORM}" == "macos" ]]; then
    docker build \
      --build-arg MACOSX_SDK="${MACOSX_SDK:-15.5}" \
      --build-arg MACOSX_SDK_SHA256="${MACOSX_SDK_SHA256:-c15cf0f3f17d714d1aa5a642da8e118db53d79429eb015771ba816aa7c6c1cbd}" \
      --build-arg OSX_VERSION_MIN="${OSXCROSS_OSX_VERSION_MIN:-11.0}" \
      --build-arg CMAKE_VERSION="${CMAKE_VERSION:-3.28.6}" \
      -t bennugd64-macos \
      -f docker/Dockerfile.macos \
      docker/
  elif [[ "${PLATFORM}" == "tvos" || "${PLATFORM}" == "ios" ]]; then
    docker build \
      --build-arg MACOSX_SDK="${MACOSX_SDK:-15.5}" \
      --build-arg MACOSX_SDK_SHA256="${MACOSX_SDK_SHA256:-c15cf0f3f17d714d1aa5a642da8e118db53d79429eb015771ba816aa7c6c1cbd}" \
      --build-arg OSX_VERSION_MIN="${OSXCROSS_OSX_VERSION_MIN:-11.0}" \
      --build-arg CMAKE_VERSION="${CMAKE_VERSION:-3.28.6}" \
      --build-arg IPHONEOS_SDK_NAME="${IPHONEOS_SDK_NAME:-iPhoneOS17.5.sdk}" \
      --build-arg IPHONEOS_SDK_REPO="${IPHONEOS_SDK_REPO:-https://github.com/xybp888/iOS-SDKs.git}" \
      --build-arg IPHONEOS_SDK_REF="${IPHONEOS_SDK_REF:-1b92ff4a8928f582876e1d388d1381c6a0c59eb9}" \
      -t "${IMAGE}" \
      -f "docker/Dockerfile.${PLATFORM}" \
      "${ROOT}"
  else
    docker build -t "${IMAGE}" -f "docker/Dockerfile.${PLATFORM}" docker/
  fi
fi

if [[ "${SECOND}" == "shell" ]]; then
  if [[ "${PLATFORM}" == "android" || "${PLATFORM}" == "switch" || "${PLATFORM}" == "dreamcast" || "${PLATFORM}" == "psp" || "${PLATFORM}" == "ps2" || "${PLATFORM}" == "ps3" || "${PLATFORM}" == "pandora" || "${PLATFORM}" == "wii" ]]; then
    exec docker run --platform linux/amd64 --rm -it \
      -v "${ROOT}:/src" \
      -w /src \
      -e HOME=/tmp \
      "${IMAGE}" bash
  fi
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

# GitHub archive tarball with retries (FetchContent's one-shot curl hits 429).
# Prefer a copy already fetched by another local/CI build of the same versions.
reuse_fetchcontent_src() {
  local dest="$1" name cand
  if [[ -f "${dest}/CMakeLists.txt" ]]; then
    return 0
  fi
  name="$(basename "${dest}")"
  for cand in \
    "${ROOT}/build-linux-static/_deps/${name}" \
    "${ROOT}/build-linux-shared/_deps/${name}" \
    "${ROOT}/build-windows-static/_deps/${name}" \
    "${ROOT}/build-windows-shared/_deps/${name}" \
    "${ROOT}/build-macos-x86_64-static/_deps/${name}" \
    "${ROOT}/build-macos-arm64-static/_deps/${name}" \
    "${ROOT}/build-android-arm64/_deps/${name}" \
    "${ROOT}/build-host/_deps/${name}" \
    "${ROOT}/build-psp-host/_deps/${name}" \
    "${ROOT}/build-vita-host/_deps/${name}" \
    "${ROOT}/build-tvos-host/_deps/${name}" \
    "${ROOT}/build-ios-host/_deps/${name}" \
    "${ROOT}/build-ps3-host/_deps/${name}" \
    "${ROOT}/build-pandora-host/_deps/${name}"
  do
    if [[ -f "${cand}/CMakeLists.txt" ]]; then
      echo "reuse: ${cand} -> ${dest}"
      rm -rf "${dest}"
      cp -a "${cand}" "${dest}"
      return 0
    fi
  done
  return 1
}

prefetch_github_archive() {
  local dest="$1" url="$2" tmp tarball top
  reuse_fetchcontent_src "${dest}" && return 0
  mkdir -p "$(dirname "${dest}")"
  tmp="$(mktemp -d "${TMPDIR:-/tmp}/bennugd-fetch.XXXXXX")"
  tarball="${tmp}/src.tgz"
  echo "prefetch: ${url}"
  curl -fsSL --retry 12 --retry-all-errors --retry-delay 5 -o "${tarball}" "${url}"
  tar -xzf "${tarball}" -C "${tmp}"
  top="$(find "${tmp}" -mindepth 1 -maxdepth 1 -type d | head -n 1)"
  rm -rf "${dest}"
  mv "${top}" "${dest}"
  rm -rf "${tmp}"
}

if [[ "${PLATFORM}" == "tvos" || "${PLATFORM}" == "ios" ]]; then
  case "${SECOND}" in
    static|shared|device|"") ;;
    simulator)
      if [[ "$(uname -s)" == "Darwin" ]]; then
      if ! command -v cmake >/dev/null 2>&1 || ! xcrun --find xcodebuild >/dev/null 2>&1; then
        echo "cmake and Xcode are required for ${PLATFORM} Simulator builds." >&2
        exit 1
      fi
      HOST_PRESET="${PLATFORM}-host"
      SIM_PRESET="${PLATFORM}-simulator"
      HOST_BUILD="${ROOT}/build-${PLATFORM}-host"
      SIM_BUILD="${ROOT}/build-${PLATFORM}-simulator"
      STAGE="${ROOT}/dist/${PLATFORM}-simulator-arm64-static"
      CONTENTS="${ROOT}/${PLATFORM}/contents"
      echo "preset: ${HOST_PRESET} + ${SIM_PRESET} (Xcode, not Docker)"
      echo "version: ${BENNUGD_VERSION}"
      rm -f "${HOST_BUILD}/CMakeCache.txt"
      rm -rf "${HOST_BUILD}/CMakeFiles"
      rm -f "${SIM_BUILD}/CMakeCache.txt"
      rm -rf "${SIM_BUILD}/CMakeFiles"
      scrub_fetchcontent "${HOST_BUILD}/_deps"
      scrub_fetchcontent "${SIM_BUILD}/_deps"
      COMMON=(
        -DBENNUGD_VERSION="${BENNUGD_VERSION}"
        -DBENNUGD_ZLIB_VERSION="${ZLIB_VERSION:-1.3.1}"
        -DBENNUGD_LIBPNG_VERSION="${LIBPNG_VERSION:-1.6.47}"
        -DBENNUGD_SDL3_REF="${SDL3_REF:-release-3.4.14}"
        -DBENNUGD_SDL3_MIXER_REF="${SDL3_MIXER_REF:-release-3.2.4}"
      )
      if [[ -n "${BENNUGD_BUNDLE_IDENTIFIER:-}" ]]; then
        COMMON+=(-DBENNUGD_BUNDLE_IDENTIFIER="${BENNUGD_BUNDLE_IDENTIFIER}")
      fi
      if [[ -n "${BENNUGD_BUNDLE_NAME:-}" ]]; then
        COMMON+=(-DBENNUGD_BUNDLE_NAME="${BENNUGD_BUNDLE_NAME}")
      fi
      cmake --preset "${HOST_PRESET}" "${COMMON[@]}"
      cmake --build --preset "${HOST_PRESET}"
      BGDC="${HOST_BUILD}/core/bgdc/src/bgdc"
      test -x "${BGDC}"
      for prg in "${ROOT}/web/demo/"*.prg; do
        dcb="${prg%.prg}.dcb"
        "${BGDC}" -o "${dcb}" "${prg}"
        test -s "${dcb}"
      done
      mkdir -p "${CONTENTS}"
      cp "${ROOT}/web/demo/hello.dcb" "${CONTENTS}/main.dcb"
      cmake --preset "${SIM_PRESET}" "${COMMON[@]}"
      cmake --build --preset "${SIM_PRESET}"
      mkdir -p "${STAGE}"
      cmake --install "${SIM_BUILD}" --prefix "${STAGE}" --config Debug
      if [[ ! -f "${STAGE}/bgdi.app/bgdi" ]]; then
        APP=""
        for cand in \
          "${SIM_BUILD}/core/bgdi/src/Debug-appletvsimulator/bgdi.app" \
          "${SIM_BUILD}/core/bgdi/src/Debug-iphonesimulator/bgdi.app" \
          "${SIM_BUILD}/core/bgdi/src/Debug/bgdi.app"
        do
          if [[ -d "${cand}" ]]; then
            APP="${cand}"
            break
          fi
        done
        if [[ -z "${APP}" ]]; then
          APP="$(find "${SIM_BUILD}/core/bgdi" -name bgdi.app -type d | grep -v /CMakeFiles/ | head -n 1 || true)"
        fi
        test -n "${APP}"
        rm -rf "${STAGE}/bgdi.app"
        cp -R "${APP}" "${STAGE}/"
      fi
      test -d "${STAGE}/bgdi.app"
      test -f "${STAGE}/bgdi.app/bgdi"
      chmod +x "${STAGE}/bgdi.app/bgdi"
      test -s "${STAGE}/bgdi.app/main.dcb"
      cp "${ROOT}/scripts/apple/sim-install.sh" "${STAGE}/sim-install.sh"
      chmod +x "${STAGE}/sim-install.sh"
      echo "Install ${STAGE}/bgdi.app in the ${PLATFORM} Simulator."
      echo "If it opens and dies: bash scripts/apple/sim-install.sh ${STAGE}/bgdi.app"
      echo "dist/${PLATFORM}-arm64-static/bgdi.app is a device binary and will not run there."
      exit 0
      fi
      ;;
    *)
      echo "${USAGE}" >&2
      exit 1
      ;;
  esac
  HOST_PRESET="${PLATFORM}-host"
  CROSS_PRESET="${PLATFORM}-arm64"
  HOST_BUILD="/src/build-${PLATFORM}-host"
  CROSS_BUILD="/src/build-${PLATFORM}-arm64"
  STAGE="/src/dist/${PLATFORM}-arm64-static"
  CONTENTS="/src/${PLATFORM}/contents"
  SDK_ENV="APPLETVOS_SDK"
  SDK_PATH="/opt/apple/AppleTVOS.sdk"
  if [[ "${PLATFORM}" == "ios" ]]; then
    SDK_ENV="IPHONEOS_SDK"
    SDK_PATH="/opt/apple/iPhoneOS.sdk"
  fi
  if [[ "${SECOND}" == "simulator" ]]; then
    CROSS_PRESET="${PLATFORM}-simulator-arm64"
    CROSS_BUILD="/src/build-${PLATFORM}-simulator-arm64"
    STAGE="/src/dist/${PLATFORM}-simulator-arm64-static"
    SDK_ENV="APPLETVSIMULATOR_SDK"
    SDK_PATH="/opt/apple/AppleTVSimulator.sdk"
    if [[ "${PLATFORM}" == "ios" ]]; then
      SDK_ENV="IPHONESIMULATOR_SDK"
      SDK_PATH="/opt/apple/iPhoneSimulator.sdk"
    fi
  fi
  echo "image: ${IMAGE}"
  echo "preset: ${HOST_PRESET} + ${CROSS_PRESET}"
  echo "version: ${BENNUGD_VERSION}"
  # Host configure on macOS stores /Users/... in the cache; Docker mounts the
  # repo at /src (same as macos/wasm).
  rm -f "${ROOT}/build-${PLATFORM}-host/CMakeCache.txt"
  rm -rf "${ROOT}/build-${PLATFORM}-host/CMakeFiles"
  rm -f "${ROOT}/${CROSS_BUILD#/src/}/CMakeCache.txt"
  rm -rf "${ROOT}/${CROSS_BUILD#/src/}/CMakeFiles"
  scrub_fetchcontent "${ROOT}/build-${PLATFORM}-host/_deps"
  scrub_fetchcontent "${ROOT}/${CROSS_BUILD#/src/}/_deps"
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
    -e BENNUGD_BUNDLE_IDENTIFIER="${BENNUGD_BUNDLE_IDENTIFIER:-}" \
    -e BENNUGD_BUNDLE_NAME="${BENNUGD_BUNDLE_NAME:-}" \
    -e HOST_PRESET="${HOST_PRESET}" \
    -e CROSS_PRESET="${CROSS_PRESET}" \
    -e HOST_BUILD="${HOST_BUILD}" \
    -e CROSS_BUILD="${CROSS_BUILD}" \
    -e STAGE="${STAGE}" \
    -e CONTENTS="${CONTENTS}" \
    -e "${SDK_ENV}=${SDK_PATH}" \
    -e CROSS_SDKROOT="${SDK_PATH}" \
    -e SDKROOT="${SDK_PATH}" \
    "${IMAGE}" \
    bash -c 'set -euo pipefail
      unset CC CXX CFLAGS CXXFLAGS
      COMMON=(
        -DBENNUGD_VERSION="${BENNUGD_VERSION}"
        -DBENNUGD_ZLIB_VERSION="${ZLIB_VERSION}"
        -DBENNUGD_LIBPNG_VERSION="${LIBPNG_VERSION}"
        -DBENNUGD_SDL3_REF="${SDL3_REF}"
        -DBENNUGD_SDL3_MIXER_REF="${SDL3_MIXER_REF}"
      )
      if [[ -n "${BENNUGD_BUNDLE_IDENTIFIER:-}" ]]; then
        COMMON+=(-DBENNUGD_BUNDLE_IDENTIFIER="${BENNUGD_BUNDLE_IDENTIFIER}")
      fi
      if [[ -n "${BENNUGD_BUNDLE_NAME:-}" ]]; then
        COMMON+=(-DBENNUGD_BUNDLE_NAME="${BENNUGD_BUNDLE_NAME}")
      fi
      cmake --preset "${HOST_PRESET}" "${COMMON[@]}"
      cmake --build --preset "${HOST_PRESET}"
      BGDC="${HOST_BUILD}/core/bgdc/src/bgdc"
      test -x "${BGDC}"
      for prg in /src/web/demo/*.prg; do
        dcb="${prg%.prg}.dcb"
        "${BGDC}" -o "${dcb}" "${prg}"
        test -s "${dcb}"
      done
      mkdir -p "${CONTENTS}"
      cp /src/web/demo/hello.dcb "${CONTENTS}/main.dcb"
      if command -v osxcross-conf >/dev/null; then
        eval "$(osxcross-conf)"
      fi
      export OSXCROSS_TARGET_DIR="${OSXCROSS_TARGET_DIR:-/opt/osxcross/target}"
      unset MACOSX_DEPLOYMENT_TARGET || true
      unset OSX_VERSION_MIN || true
      export SDKROOT="${CROSS_SDKROOT:-${SDKROOT:-}}"
      cmake --preset "${CROSS_PRESET}" "${COMMON[@]}"
      cmake --build --preset "${CROSS_PRESET}"
      mkdir -p "${STAGE}"
      cmake --install "${CROSS_BUILD}" --prefix "${STAGE}"
      if [[ ! -d "${STAGE}/bgdi.app" ]]; then
        APP=""
        for cand in \
          "${CROSS_BUILD}/core/bgdi/src/bgdi.app" \
          "${CROSS_BUILD}/core/bgdi/src/Release/bgdi.app"
        do
          if [[ -d "${cand}" ]]; then
            APP="${cand}"
            break
          fi
        done
        if [[ -z "${APP}" ]]; then
          APP="$(find "${CROSS_BUILD}/core/bgdi" -name bgdi.app -type d | grep -v /CMakeFiles/ | head -n 1 || true)"
        fi
        test -n "${APP}"
        cp -R "${APP}" "${STAGE}/"
      fi
      test -d "${STAGE}/bgdi.app"
      test -f "${STAGE}/bgdi.app/bgdi"
      chmod +x "${STAGE}/bgdi.app/bgdi"
      test -s "${STAGE}/bgdi.app/main.dcb"
      if [[ "${STAGE}" == *simulator* ]]; then
        cp /src/scripts/apple/sim-install.sh "${STAGE}/sim-install.sh"
        chmod +x "${STAGE}/sim-install.sh"
      fi
    '
  test -d "${ROOT}/${STAGE#/src/}/bgdi.app"
  if [[ "${SECOND}" == "simulator" ]]; then
    echo "Simulator .app: ${STAGE#/src/}/bgdi.app (osxcross). On a Mac: bash sim-install.sh (in the extracted folder)."
  else
    echo "Device .app: dist/${PLATFORM}-arm64-static/bgdi.app (sign on a Mac)."
    echo "That binary is appletvos/iphoneos — it will not run in Simulator."
    echo "For Simulator: bash scripts/build.sh ${PLATFORM} simulator"
  fi
  exit 0
fi

if [[ "${PLATFORM}" == "wasm" ]]; then
  echo "image: ${IMAGE}"
  echo "preset: wasm-host + emcmake + wasi"
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
    -e WASI_SDK_VERSION="${WASI_SDK_VERSION:-33}" \
    -e WASI_SDK_VERSION_FULL="${WASI_SDK_VERSION_FULL:-33.0}" \
    "${IMAGE}" \
    bash -c 'set -euo pipefail
      set +eu
      # shellcheck disable=SC1091
      source "${EMSDK}/emsdk_env.sh"
      set -euo pipefail
      unset CC CXX CFLAGS CXXFLAGS
      export EM_CACHE="${HOME}/emscripten-cache"
      mkdir -p "${EM_CACHE}"
      HOST_BUILD=/src/build-host
      WASM_BUILD=/src/build-wasm
      STAGE=/src/dist/web-wasm32-static
      FETCH_DIR="${HOST_BUILD}/_deps"
      COMMON=(
        -DBENNUGD_VERSION="${BENNUGD_VERSION}"
        -DBENNUGD_ZLIB_VERSION="${ZLIB_VERSION}"
        -DBENNUGD_LIBPNG_VERSION="${LIBPNG_VERSION}"
        -DBENNUGD_SDL3_REF="${SDL3_REF}"
        -DBENNUGD_SDL3_MIXER_REF="${SDL3_MIXER_REF}"
      )
      emcc -v
      cmake --preset wasm-host \
        "${COMMON[@]}" \
        -DFETCHCONTENT_BASE_DIR="${FETCH_DIR}"
      cmake --build --preset wasm-host
      for prg in /src/web/demo/*.prg; do
        dcb="${prg%.prg}.dcb"
        "${HOST_BUILD}/core/bgdc/src/bgdc" -o "${dcb}" "${prg}"
        test -s "${dcb}"
      done
      # Drop a cache from a previous EMSDK layout (/opt/emsdk vs /emsdk).
      rm -f "${WASM_BUILD}/CMakeCache.txt"
      rm -rf "${WASM_BUILD}/CMakeFiles"
      emcmake cmake -S /src -B "${WASM_BUILD}" -G Ninja \
        -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -DUSE_LIBDES=ON \
        -DBENNUGD_BUNDLE_DEPS=ON \
        -DSTATIC_MODULES=ON \
        -DINTERPRETER_ONLY=ON \
        "${COMMON[@]}" \
        -DFETCHCONTENT_BASE_DIR="${WASM_BUILD}/_deps" \
        -DFETCHCONTENT_SOURCE_DIR_ZLIB="${FETCH_DIR}/zlib-src"
      cmake --build "${WASM_BUILD}" --target bgdi
      unset CC CXX CFLAGS CXXFLAGS CMAKE_TOOLCHAIN_FILE
      # Drop a failed/wrong-arch cache (e.g. x86_64 wasi-sdk on arm64 OrbStack).
      rm -f /src/build-wasi/CMakeCache.txt
      rm -rf /src/build-wasi/CMakeFiles
      cmake --preset wasi \
        "${COMMON[@]}" \
        -DBENNUGD_WASI_SDK_VERSION="${WASI_SDK_VERSION:-33}" \
        -DBENNUGD_WASI_SDK_VERSION_FULL="${WASI_SDK_VERSION_FULL:-33.0}" \
        -DFETCHCONTENT_BASE_DIR=/src/build-wasi/_deps \
        -DFETCHCONTENT_SOURCE_DIR_ZLIB="${FETCH_DIR}/zlib-src"
      cmake --build --preset wasi
      SRC="${WASM_BUILD}/core/bgdi/src"
      mkdir -p "${STAGE}/ide" "${STAGE}/samples"
      cp "${SRC}/bgdi.html" "${STAGE}/index.html"
      cp "${SRC}/bgdi.html" "${SRC}/bgdi.js" "${SRC}/bgdi.wasm" "${SRC}/bgdi.data" "${STAGE}/"
      cp /src/build-wasi/core/bgdc/src/bgdc.wasm "${STAGE}/bgdc.wasm"
      cp -a /src/web/ide/. "${STAGE}/ide/"
      cp /src/web/demo/*.prg "${STAGE}/samples/"
      cp /src/README.md "${STAGE}/"
      cp "${WASM_BUILD}/BUILD_INFO.txt" "${STAGE}/"
      cp "${SRC}/bgdi.wasm.map" "${STAGE}/" 2>/dev/null || true
      cp "${SRC}/bgdi.js.map" "${STAGE}/" 2>/dev/null || true
      test -s "${STAGE}/index.html"
      test -s "${STAGE}/bgdi.html"
      test -s "${STAGE}/bgdi.js"
      test -s "${STAGE}/bgdi.wasm"
      test -s "${STAGE}/bgdi.data"
      test -s "${STAGE}/bgdc.wasm"
      test -s "${STAGE}/ide/index.html"
      test -s "${STAGE}/samples/hello.prg"
    '
  exit 0
fi

if [[ "${PLATFORM}" == "android" ]]; then
  echo "image: ${IMAGE}"
  echo "preset: android-host + android-arm64"
  echo "version: ${BENNUGD_VERSION}"
  scrub_fetchcontent "${ROOT}/build-android-host/_deps"
  scrub_fetchcontent "${ROOT}/build-android-arm64/_deps"
  docker run --platform linux/amd64 --rm \
    -u "$(id -u):$(id -g)" \
    -v "${ROOT}:/src" \
    -w /src \
    -e HOME=/tmp \
    -e GRADLE_USER_HOME=/src/build-android-gradle \
    -e ANDROID_USER_HOME=/tmp/android \
    -e BENNUGD_VERSION="${BENNUGD_VERSION}" \
    -e BUILD_TYPE="${BUILD_TYPE:-Release}" \
    -e ZLIB_VERSION="${ZLIB_VERSION:-1.3.1}" \
    -e LIBPNG_VERSION="${LIBPNG_VERSION:-1.6.47}" \
    -e SDL3_REF="${SDL3_REF:-release-3.4.14}" \
    -e SDL3_MIXER_REF="${SDL3_MIXER_REF:-release-3.2.4}" \
    -e ANDROID_API="${ANDROID_API:-28}" \
    "${IMAGE}" \
    bash -c 'set -euo pipefail
      HOST_BUILD=/src/build-android-host
      NDK_BUILD=/src/build-android-arm64
      APK_WORK=/src/build-android-apk
      STAGE=/src/dist/android-arm64-static
      FETCH_DIR="${HOST_BUILD}/_deps"
      JNI="${APK_WORK}/app/src/main/jniLibs/arm64-v8a"
      COMMON=(
        -DBENNUGD_VERSION="${BENNUGD_VERSION}"
        -DBENNUGD_ZLIB_VERSION="${ZLIB_VERSION}"
        -DBENNUGD_LIBPNG_VERSION="${LIBPNG_VERSION}"
        -DBENNUGD_SDL3_REF="${SDL3_REF}"
        -DBENNUGD_SDL3_MIXER_REF="${SDL3_MIXER_REF}"
      )
      cmake --preset android-host "${COMMON[@]}"
      cmake --build --preset android-host
      for prg in /src/web/demo/*.prg; do
        dcb="${prg%.prg}.dcb"
        "${HOST_BUILD}/core/bgdc/src/bgdc" -o "${dcb}" "${prg}"
        test -s "${dcb}"
      done
      test -n "${ANDROID_NDK:-}"
      test -f "${ANDROID_NDK}/build/cmake/android.toolchain.cmake"
      cmake --preset android-arm64 \
        "${COMMON[@]}" \
        -DANDROID_ABI=arm64-v8a \
        -DANDROID_PLATFORM="android-${ANDROID_API}" \
        -DANDROID_STL=c++_shared \
        -DFETCHCONTENT_SOURCE_DIR_ZLIB="${FETCH_DIR}/zlib-src"
      cmake --build --preset android-arm64
      rm -rf "${APK_WORK}"
      mkdir -p "${APK_WORK}" "${JNI}" "${APK_WORK}/app/src/main/assets" "${STAGE}"
      cp -a /src/android/. "${APK_WORK}/"
      rm -rf "${APK_WORK}/.gradle" "${APK_WORK}/app/build" "${APK_WORK}/build"
      JAVA_SRC="${NDK_BUILD}/_deps/sdl3-src/android-project/app/src/main/java"
      test -d "${JAVA_SRC}"
      mkdir -p "${APK_WORK}/app/src/main"
      cp -a "${JAVA_SRC}" "${APK_WORK}/app/src/main/"
      mkdir -p "${JNI}"
      copy_jni() {
        local name="$1" required="${2:-1}" search="${3:-${NDK_BUILD}}" f="" cand
        for cand in \
          "${search}/${name}" \
          "${search}/src/${name}" \
          "${search}/lib/${name}"
        do
          if [[ -e "${cand}" ]]; then
            f="${cand}"
            break
          fi
        done
        if [[ -z "${f}" ]]; then
          f="$(find "${search}" \( -type f -o -type l \) -name "${name}" | grep -v /CMakeFiles/ | head -n 1 || true)"
        fi
        if [[ -z "${f}" ]]; then
          if [[ "${required}" == "1" ]]; then
            echo "missing ${name} under ${search}" >&2
            exit 1
          fi
          return 0
        fi
        cp -L "${f}" "${JNI}/"
      }
      copy_jni libmain.so 1 "${NDK_BUILD}/core/bgdi"
      copy_jni libSDL3.so 1 "${NDK_BUILD}/_deps/sdl3-build"
      copy_jni libhidapi.so 0 "${NDK_BUILD}/_deps/sdl3-build"
      CXX_SO=""
      PREBUILT=""
      for host in linux-x86_64 linux-aarch64; do
        if [[ -d "${ANDROID_NDK}/toolchains/llvm/prebuilt/${host}" ]]; then
          PREBUILT="${ANDROID_NDK}/toolchains/llvm/prebuilt/${host}"
          break
        fi
      done
      if [[ -z "${PREBUILT}" ]]; then
        echo "missing NDK llvm prebuilt under ${ANDROID_NDK}/toolchains/llvm/prebuilt" >&2
        exit 1
      fi
      for cand in \
        "${PREBUILT}/sysroot/usr/lib/aarch64-linux-android/libc++_shared.so" \
        "${PREBUILT}/sysroot/usr/lib/aarch64-linux-android/${ANDROID_API}/libc++_shared.so"
      do
        if [[ -f "${cand}" ]]; then
          CXX_SO="${cand}"
          break
        fi
      done
      if [[ -z "${CXX_SO}" ]]; then
        echo "missing libc++_shared.so in ${PREBUILT}" >&2
        exit 1
      fi
      cp -L "${CXX_SO}" "${JNI}/"
      mkdir -p "${APK_WORK}/app/src/main/assets"
      cp /src/web/demo/*.dcb "${APK_WORK}/app/src/main/assets/"
      cp /src/web/demo/hello.dcb "${APK_WORK}/app/src/main/assets/main.dcb"
      printf "sdk.dir=%s\n" "${ANDROID_HOME}" > "${APK_WORK}/local.properties"
      mkdir -p "${GRADLE_USER_HOME}" "${HOME}" "${ANDROID_USER_HOME:-/tmp/android}"
      ( cd "${APK_WORK}" && gradle --no-daemon --no-watch-fs assembleDebug )
      APK="$(echo "${APK_WORK}"/app/build/outputs/apk/debug/*.apk)"
      test -s "${APK}"
      cmake --install "${NDK_BUILD}" --prefix "${STAGE}"
      cp "${APK}" "${STAGE}/bennugd64.apk"
      cp "${JNI}/"* "${STAGE}/" 2>/dev/null || true
      test -s "${STAGE}/bennugd64.apk"
      test -s "${STAGE}/libmain.so"
    '
  exit 0
fi

if [[ "${PLATFORM}" == "switch" ]]; then
  echo "image: ${IMAGE}"
  echo "preset: switch-host + switch-aarch64"
  echo "version: ${BENNUGD_VERSION}"
  scrub_fetchcontent "${ROOT}/build-switch-host/_deps"
  scrub_fetchcontent "${ROOT}/build-switch-aarch64/_deps"
  docker run --platform linux/amd64 --rm \
    -u "$(id -u):$(id -g)" \
    -v "${ROOT}:/src" \
    -w /src \
    -e HOME=/tmp \
    -e BENNUGD_VERSION="${BENNUGD_VERSION}" \
    -e BUILD_TYPE="${BUILD_TYPE:-Release}" \
    -e ZLIB_VERSION="${ZLIB_VERSION:-1.3.1}" \
    -e LIBPNG_VERSION="${LIBPNG_VERSION:-1.6.47}" \
    -e SDL3_REF="${SDL3_REF:-release-3.4.14}" \
    -e SDL3_SWITCH_REF="${SDL3_SWITCH_REF:-switch-sdl-3.4}" \
    -e SDL3_MIXER_REF="${SDL3_MIXER_REF:-release-3.2.4}" \
    "${IMAGE}" \
    bash -c 'set -euo pipefail
      unset CC CXX CFLAGS CXXFLAGS
      test -n "${DEVKITPRO:-}"
      test -f "${DEVKITPRO}/libnx/switch.specs"
      HOST_BUILD=/src/build-switch-host
      NX_BUILD=/src/build-switch-aarch64
      ROMFS=/src/build-switch-romfs
      STAGE=/src/dist/switch-aarch64-static
      FETCH_DIR="${HOST_BUILD}/_deps"
      COMMON=(
        -DBENNUGD_VERSION="${BENNUGD_VERSION}"
        -DBENNUGD_ZLIB_VERSION="${ZLIB_VERSION}"
        -DBENNUGD_LIBPNG_VERSION="${LIBPNG_VERSION}"
        -DBENNUGD_SDL3_REF="${SDL3_REF}"
        -DBENNUGD_SDL3_SWITCH_REF="${SDL3_SWITCH_REF}"
        -DBENNUGD_SDL3_MIXER_REF="${SDL3_MIXER_REF}"
      )
      cmake --preset switch-host "${COMMON[@]}"
      cmake --build --preset switch-host
      for prg in /src/web/demo/*.prg; do
        dcb="${prg%.prg}.dcb"
        "${HOST_BUILD}/core/bgdc/src/bgdc" -o "${dcb}" "${prg}"
        test -s "${dcb}"
      done
      cmake --preset switch-aarch64 \
        "${COMMON[@]}" \
        -DFETCHCONTENT_SOURCE_DIR_ZLIB="${FETCH_DIR}/zlib-src"
      cmake --build --preset switch-aarch64
      ELF=""
      for cand in \
        "${NX_BUILD}/core/bgdi/src/bgdi.elf" \
        "${NX_BUILD}/core/bgdi/src/bgdi"
      do
        if [[ -f "${cand}" ]]; then
          ELF="${cand}"
          break
        fi
      done
      if [[ -z "${ELF}" ]]; then
        ELF="$(find "${NX_BUILD}/core/bgdi" \( -type f -o -type l \) \( -name bgdi.elf -o -name bgdi \) | grep -v /CMakeFiles/ | head -n 1 || true)"
      fi
      test -n "${ELF}"
      test -s "${ELF}"
      rm -rf "${ROMFS}"
      mkdir -p "${ROMFS}" "${STAGE}"
      cp /src/web/demo/*.dcb "${ROMFS}/"
      cp /src/web/demo/hello.dcb "${ROMFS}/main.dcb"
      NACP="${NX_BUILD}/bennugd64.nacp"
      NRO="${STAGE}/bennugd64.nro"
      nacptool --create "BennuGD64" "BennuGD64" "${BENNUGD_VERSION}" "${NACP}"
      ELF2NRO=(elf2nro "${ELF}" "${NRO}" --nacp="${NACP}" --romfsdir="${ROMFS}")
      if [[ -f "${DEVKITPRO}/libnx/default_icon.jpg" ]]; then
        ELF2NRO+=(--icon="${DEVKITPRO}/libnx/default_icon.jpg")
      fi
      "${ELF2NRO[@]}"
      cmake --install "${NX_BUILD}" --prefix "${STAGE}"
      cp "${ELF}" "${STAGE}/bgdi.elf"
      test -s "${STAGE}/bennugd64.nro"
      test -s "${STAGE}/bgdi.elf"
    '
  exit 0
fi

if [[ "${PLATFORM}" == "dreamcast" ]]; then
  echo "image: ${IMAGE}"
  echo "preset: dreamcast-host + dreamcast-sh4"
  echo "version: ${BENNUGD_VERSION}"
  scrub_fetchcontent "${ROOT}/build-dreamcast-host/_deps"
  scrub_fetchcontent "${ROOT}/build-dreamcast-sh4/_deps"
  docker run --platform linux/amd64 --rm \
    -u "$(id -u):$(id -g)" \
    -v "${ROOT}:/src" \
    -w /src \
    -e HOME=/tmp \
    -e BENNUGD_VERSION="${BENNUGD_VERSION}" \
    -e BUILD_TYPE="${BUILD_TYPE:-Release}" \
    -e ZLIB_VERSION="${ZLIB_VERSION:-1.3.1}" \
    -e LIBPNG_VERSION="${LIBPNG_VERSION:-1.6.47}" \
    -e SDL3_REF="${SDL3_REF:-release-3.4.14}" \
    -e SDL3_DREAMCAST_REF="${SDL3_DREAMCAST_REF:-dreamcastSDL3}" \
    -e SDL3_MIXER_REF="${SDL3_MIXER_REF:-release-3.2.4}" \
    "${IMAGE}" \
    bash -c 'set -euo pipefail
      unset CC CXX CFLAGS CXXFLAGS
      test -n "${KOS_BASE:-}"
      test -x "${KOS_BASE}/utils/build_wrappers/kos-cc" \
        -o -x "${KOS_BASE}/utils/gnu_wrappers/kos-cc"
      HOST_BUILD=/src/build-dreamcast-host
      DC_BUILD=/src/build-dreamcast-sh4
      ISODIR=/src/build-dreamcast-iso
      STAGE=/src/dist/dreamcast-sh4-static
      FETCH_DIR="${HOST_BUILD}/_deps"
      COMMON=(
        -DBENNUGD_VERSION="${BENNUGD_VERSION}"
        -DBENNUGD_ZLIB_VERSION="${ZLIB_VERSION}"
        -DBENNUGD_LIBPNG_VERSION="${LIBPNG_VERSION}"
        -DBENNUGD_SDL3_REF="${SDL3_REF}"
        -DBENNUGD_SDL3_DREAMCAST_REF="${SDL3_DREAMCAST_REF}"
        -DBENNUGD_SDL3_MIXER_REF="${SDL3_MIXER_REF}"
      )
      cmake --preset dreamcast-host "${COMMON[@]}"
      cmake --build --preset dreamcast-host
      for prg in /src/web/demo/*.prg; do
        dcb="${prg%.prg}.dcb"
        "${HOST_BUILD}/core/bgdc/src/bgdc" -o "${dcb}" "${prg}"
        test -s "${dcb}"
      done
      cmake --preset dreamcast-sh4 \
        "${COMMON[@]}" \
        -DFETCHCONTENT_SOURCE_DIR_ZLIB="${FETCH_DIR}/zlib-src"
      cmake --build --preset dreamcast-sh4
      ELF=""
      for cand in \
        "${DC_BUILD}/core/bgdi/src/bgdi.elf" \
        "${DC_BUILD}/core/bgdi/src/bgdi"
      do
        if [[ -f "${cand}" ]]; then
          ELF="${cand}"
          break
        fi
      done
      if [[ -z "${ELF}" ]]; then
        ELF="$(find "${DC_BUILD}/core/bgdi" \( -type f -o -type l \) \( -name bgdi.elf -o -name bgdi \) | grep -v /CMakeFiles/ | head -n 1 || true)"
      fi
      test -n "${ELF}"
      test -s "${ELF}"
      rm -rf "${ISODIR}"
      mkdir -p "${ISODIR}" "${STAGE}"
      cp /src/web/demo/*.dcb "${ISODIR}/"
      cp /src/web/demo/hello.dcb "${ISODIR}/main.dcb"
      CDI="${STAGE}/bennugd64.cdi"
      MKDCDISC="$(command -v mkdcdisc || true)"
      if [[ -z "${MKDCDISC}" ]]; then
        for cand in /opt/toolchains/dc/bin/mkdcdisc /usr/bin/mkdcdisc; do
          if [[ -x "${cand}" ]]; then
            MKDCDISC="${cand}"
            break
          fi
        done
      fi
      test -n "${MKDCDISC}"
      "${MKDCDISC}" -n "BennuGD64" -a "BennuGD64" -N -e "${ELF}" -D "${ISODIR}" -o "${CDI}"
      cmake --install "${DC_BUILD}" --prefix "${STAGE}"
      cp "${ELF}" "${STAGE}/bgdi.elf"
      cp "${ISODIR}/"* "${STAGE}/" 2>/dev/null || true
      test -s "${STAGE}/bennugd64.cdi"
      test -s "${STAGE}/bgdi.elf"
    '
  exit 0
fi

if [[ "${PLATFORM}" == "psp" ]]; then
  echo "image: ${IMAGE}"
  echo "preset: psp-host + psp-mips"
  echo "version: ${BENNUGD_VERSION}"
  scrub_fetchcontent "${ROOT}/build-psp-host/_deps"
  scrub_fetchcontent "${ROOT}/build-psp-mips/_deps"
  docker run --platform linux/amd64 --rm \
    -u "$(id -u):$(id -g)" \
    -v "${ROOT}:/src" \
    -w /src \
    -e HOME=/tmp \
    -e BENNUGD_VERSION="${BENNUGD_VERSION}" \
    -e BUILD_TYPE="${BUILD_TYPE:-Release}" \
    -e ZLIB_VERSION="${ZLIB_VERSION:-1.3.1}" \
    -e LIBPNG_VERSION="${LIBPNG_VERSION:-1.6.47}" \
    "${IMAGE}" \
    bash -c 'set -euo pipefail
      unset CC CXX CFLAGS CXXFLAGS
      test -n "${PSPDEV:-}"
      test -x "${PSPDEV}/bin/psp-gcc"
      test -x "${PSPDEV}/bin/pack-pbp"
      HOST_BUILD=/src/build-psp-host
      PSP_BUILD=/src/build-psp-mips
      GAMEDIR=/src/build-psp-game
      STAGE=/src/dist/psp-mips-static
      FETCH_DIR="${HOST_BUILD}/_deps"
      COMMON=(
        -DBENNUGD_VERSION="${BENNUGD_VERSION}"
        -DBENNUGD_ZLIB_VERSION="${ZLIB_VERSION}"
        -DBENNUGD_LIBPNG_VERSION="${LIBPNG_VERSION}"
      )
      cmake --preset psp-host "${COMMON[@]}"
      cmake --build --preset psp-host
      for prg in /src/web/demo/*.prg; do
        dcb="${prg%.prg}.dcb"
        "${HOST_BUILD}/core/bgdc/src/bgdc" -o "${dcb}" "${prg}"
        test -s "${dcb}"
      done
      cmake --preset psp-mips \
        "${COMMON[@]}" \
        -DFETCHCONTENT_SOURCE_DIR_ZLIB="${FETCH_DIR}/zlib-src"
      cmake --build --preset psp-mips
      ELF=""
      for cand in \
        "${PSP_BUILD}/core/bgdi/src/bgdi.elf" \
        "${PSP_BUILD}/core/bgdi/src/bgdi"
      do
        if [[ -f "${cand}" ]]; then
          ELF="${cand}"
          break
        fi
      done
      if [[ -z "${ELF}" ]]; then
        ELF="$(find "${PSP_BUILD}/core/bgdi" \( -type f -o -type l \) \( -name bgdi.elf -o -name bgdi \) | grep -v /CMakeFiles/ | head -n 1 || true)"
      fi
      test -n "${ELF}"
      test -s "${ELF}"
      rm -rf "${GAMEDIR}"
      mkdir -p "${GAMEDIR}" "${STAGE}"
      cp "${ELF}" "${GAMEDIR}/bgdi.elf"
      "${PSPDEV}/bin/psp-fixup-imports" "${GAMEDIR}/bgdi.elf"
      "${PSPDEV}/bin/mksfoex" -d MEMSIZE=1 -s "APP_VER=1.0" "BennuGD64" "${GAMEDIR}/PARAM.SFO"
      "${PSPDEV}/bin/pack-pbp" "${GAMEDIR}/EBOOT.PBP" "${GAMEDIR}/PARAM.SFO" \
        NULL NULL NULL NULL NULL "${GAMEDIR}/bgdi.elf" NULL
      cp /src/web/demo/*.dcb "${GAMEDIR}/"
      cp /src/web/demo/hello.dcb "${GAMEDIR}/main.dcb"
      cmake --install "${PSP_BUILD}" --prefix "${STAGE}"
      cp "${GAMEDIR}/EBOOT.PBP" "${GAMEDIR}/bgdi.elf" "${STAGE}/"
      cp "${GAMEDIR}/"* "${STAGE}/" 2>/dev/null || true
      test -s "${STAGE}/EBOOT.PBP"
      test -s "${STAGE}/bgdi.elf"
    '
  exit 0
fi

if [[ "${PLATFORM}" == "vita" ]]; then
  echo "image: ${IMAGE}"
  echo "preset: vita-host + vita-arm"
  echo "version: ${BENNUGD_VERSION}"
  scrub_fetchcontent "${ROOT}/build-vita-host/_deps"
  scrub_fetchcontent "${ROOT}/build-vita-arm/_deps"
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
      unset CC CXX CFLAGS CXXFLAGS
      test -n "${VITASDK:-}"
      test -x "${VITASDK}/bin/arm-vita-eabi-gcc"
      test -x "${VITASDK}/bin/vita-elf-create"
      test -x "${VITASDK}/bin/vita-make-fself"
      test -x "${VITASDK}/bin/vita-pack-vpk"
      test -x "${VITASDK}/bin/vita-mksfoex"
      HOST_BUILD=/src/build-vita-host
      VITA_BUILD=/src/build-vita-arm
      GAMEDIR=/src/build-vita-game
      STAGE=/src/dist/vita-arm-static
      FETCH_DIR="${HOST_BUILD}/_deps"
      COMMON=(
        -DBENNUGD_VERSION="${BENNUGD_VERSION}"
        -DBENNUGD_ZLIB_VERSION="${ZLIB_VERSION}"
        -DBENNUGD_LIBPNG_VERSION="${LIBPNG_VERSION}"
        -DBENNUGD_SDL3_REF="${SDL3_REF}"
        -DBENNUGD_SDL3_MIXER_REF="${SDL3_MIXER_REF}"
      )
      cmake --preset vita-host "${COMMON[@]}"
      cmake --build --preset vita-host
      for prg in /src/web/demo/*.prg; do
        dcb="${prg%.prg}.dcb"
        "${HOST_BUILD}/core/bgdc/src/bgdc" -o "${dcb}" "${prg}"
        test -s "${dcb}"
      done
      cmake --preset vita-arm \
        "${COMMON[@]}" \
        -DFETCHCONTENT_SOURCE_DIR_ZLIB="${FETCH_DIR}/zlib-src"
      cmake --build --preset vita-arm
      ELF=""
      for cand in \
        "${VITA_BUILD}/core/bgdi/src/bgdi" \
        "${VITA_BUILD}/core/bgdi/src/bgdi.elf"
      do
        if [[ -f "${cand}" ]]; then
          ELF="${cand}"
          break
        fi
      done
      if [[ -z "${ELF}" ]]; then
        ELF="$(find "${VITA_BUILD}/core/bgdi" \( -type f -o -type l \) \( -name bgdi -o -name bgdi.elf \) | grep -v /CMakeFiles/ | head -n 1 || true)"
      fi
      test -n "${ELF}"
      test -s "${ELF}"
      rm -rf "${GAMEDIR}"
      mkdir -p "${GAMEDIR}" "${STAGE}"
      cp "${ELF}" "${GAMEDIR}/bgdi.elf"
      "${VITASDK}/bin/vita-elf-create" "${GAMEDIR}/bgdi.elf" "${GAMEDIR}/bgdi.velf"
      "${VITASDK}/bin/vita-make-fself" -u "${GAMEDIR}/bgdi.velf" "${GAMEDIR}/eboot.bin"
      "${VITASDK}/bin/vita-mksfoex" -s TITLE_ID=BGDV00001 \
        -d PARENTAL_LEVEL=1 -d ATTRIBUTE2=12 \
        "BennuGD64" "${GAMEDIR}/param.sfo"
      cp /src/web/demo/*.dcb "${GAMEDIR}/"
      cp /src/web/demo/hello.dcb "${GAMEDIR}/main.dcb"
      (
        cd "${GAMEDIR}"
        "${VITASDK}/bin/vita-pack-vpk" -s param.sfo -b eboot.bin \
          -a main.dcb=main.dcb \
          "${STAGE}/bennugd64.vpk"
      )
      cmake --install "${VITA_BUILD}" --prefix "${STAGE}"
      cp "${GAMEDIR}/bgdi.elf" "${GAMEDIR}/eboot.bin" "${GAMEDIR}/main.dcb" "${STAGE}/"
      test -s "${STAGE}/bennugd64.vpk"
      test -s "${STAGE}/eboot.bin"
    '
  exit 0
fi

if [[ "${PLATFORM}" == "ps2" ]]; then
  echo "image: ${IMAGE}"
  echo "preset: ps2-host + ps2-mips"
  echo "version: ${BENNUGD_VERSION}"
  scrub_fetchcontent "${ROOT}/build-ps2-host/_deps"
  scrub_fetchcontent "${ROOT}/build-ps2-mips/_deps"
  docker run --platform linux/amd64 --rm \
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
      unset CC CXX CFLAGS CXXFLAGS
      test -n "${PS2DEV:-}"
      test -n "${PS2SDK:-}"
      test -x "${PS2DEV}/ee/bin/mips64r5900el-ps2-elf-gcc" \
        -o -x "${PS2DEV}/ee/bin/ee-gcc"
      HOST_BUILD=/src/build-ps2-host
      PS2_BUILD=/src/build-ps2-mips
      STAGE=/src/dist/ps2-mips-static
      COMMON=(
        -DBENNUGD_VERSION="${BENNUGD_VERSION}"
        -DBENNUGD_ZLIB_VERSION="${ZLIB_VERSION}"
        -DBENNUGD_LIBPNG_VERSION="${LIBPNG_VERSION}"
        -DBENNUGD_SDL3_REF="${SDL3_REF}"
        -DBENNUGD_SDL3_MIXER_REF="${SDL3_MIXER_REF}"
      )
      cmake --preset ps2-host "${COMMON[@]}"
      cmake --build --preset ps2-host
      for prg in /src/web/demo/*.prg; do
        dcb="${prg%.prg}.dcb"
        "${HOST_BUILD}/core/bgdc/src/bgdc" -o "${dcb}" "${prg}"
        test -s "${dcb}"
      done
      # Drop a cache that used FetchContent zlib/libpng (conflicts with ps2sdk-ports).
      rm -f "${PS2_BUILD}/CMakeCache.txt"
      rm -rf "${PS2_BUILD}/CMakeFiles"
      cmake --preset ps2-mips "${COMMON[@]}"
      cmake --build --preset ps2-mips
      ELF=""
      for cand in \
        "${PS2_BUILD}/core/bgdi/src/bgdi.elf" \
        "${PS2_BUILD}/core/bgdi/src/bgdi"
      do
        if [[ -f "${cand}" ]]; then
          ELF="${cand}"
          break
        fi
      done
      if [[ -z "${ELF}" ]]; then
        ELF="$(find "${PS2_BUILD}/core/bgdi" \( -type f -o -type l \) \( -name bgdi.elf -o -name bgdi \) | grep -v /CMakeFiles/ | head -n 1 || true)"
      fi
      test -n "${ELF}"
      test -s "${ELF}"
      mkdir -p "${STAGE}"
      cmake --install "${PS2_BUILD}" --prefix "${STAGE}"
      cp "${ELF}" "${STAGE}/bgdi.elf"
      STRIP="${PS2DEV}/ee/bin/mips64r5900el-ps2-elf-strip"
      if [[ -x "${STRIP}" ]]; then
        "${STRIP}" "${STAGE}/bgdi.elf" || true
      fi
      cp /src/web/demo/*.dcb "${STAGE}/"
      cp /src/web/demo/hello.dcb "${STAGE}/main.dcb"
      cp /src/ps2/SYSTEM.CNF "${STAGE}/"
      ISODIR=/src/build-ps2-iso
      rm -rf "${ISODIR}"
      mkdir -p "${ISODIR}"
      cp "${STAGE}/SYSTEM.CNF" "${ISODIR}/SYSTEM.CNF"
      cp "${STAGE}/bgdi.elf" "${ISODIR}/BGDI.ELF"
      cp "${STAGE}/main.dcb" "${ISODIR}/MAIN.DCB"
      MKISO=""
      for cand in xorrisofs mkisofs genisoimage; do
        if command -v "${cand}" >/dev/null 2>&1; then
          MKISO="${cand}"
          break
        fi
      done
      if [[ -z "${MKISO}" ]] && command -v xorriso >/dev/null 2>&1; then
        MKISO="xorriso -as mkisofs"
      fi
      test -n "${MKISO}"
      # ISO 9660 level 1 + XA: SYSTEM.CNF / BGDI.ELF / MAIN.DCB. PCSX2 File→Open the .iso.
      if ! ${MKISO} -o "${STAGE}/bennugd64.iso" \
        -iso-level 1 \
        -xa \
        -sysid PLAYSTATION \
        -V BENNUGD64 \
        -A BennuGD64 \
        "${ISODIR}"
      then
        ${MKISO} -o "${STAGE}/bennugd64.iso" \
          -iso-level 1 \
          -sysid PLAYSTATION \
          -V BENNUGD64 \
          -A BennuGD64 \
          "${ISODIR}"
      fi
      test -s "${STAGE}/bgdi.elf"
      test -s "${STAGE}/main.dcb"
      test -s "${STAGE}/SYSTEM.CNF"
      test -s "${STAGE}/bennugd64.iso"
    '
  exit 0
fi

if [[ "${PLATFORM}" == "ps3" ]]; then
  echo "image: ${IMAGE}"
  echo "preset: ps3-host + ps3-ppu"
  echo "version: ${BENNUGD_VERSION}"
  scrub_fetchcontent "${ROOT}/build-ps3-host/_deps"
  scrub_fetchcontent "${ROOT}/build-ps3-ppu/_deps"
  docker run --platform linux/amd64 --rm \
    -u "$(id -u):$(id -g)" \
    -v "${ROOT}:/src" \
    -w /src \
    -e HOME=/tmp \
    -e BENNUGD_VERSION="${BENNUGD_VERSION}" \
    -e BUILD_TYPE="${BUILD_TYPE:-Release}" \
    -e ZLIB_VERSION="${ZLIB_VERSION:-1.3.1}" \
    -e LIBPNG_VERSION="${LIBPNG_VERSION:-1.6.47}" \
    -e SDL3_REF="${SDL3_REF:-release-3.4.14}" \
    -e SDL3_PS3_REF="${SDL3_PS3_REF:-ps3}" \
    -e SDL3_MIXER_REF="${SDL3_MIXER_REF:-release-3.2.4}" \
    "${IMAGE}" \
    bash -c 'set -euo pipefail
      unset CC CXX CFLAGS CXXFLAGS
      test -n "${PS3DEV:-}"
      test -x "${PS3DEV}/ppu/bin/powerpc64-ps3-elf-gcc" \
        -o -x "${PS3DEV}/ppu/bin/ppu-gcc"
      HOST_BUILD=/src/build-ps3-host
      PS3_BUILD=/src/build-ps3-ppu
      PKGDIR=/src/build-ps3-pkg
      STAGE=/src/dist/ps3-ppu-static
      FETCH_DIR="${HOST_BUILD}/_deps"
      GAME_TITLE=BennuGD64
      GAME_ID=BGD300001
      CONTENT_ID=UP0000-BGD300001_00-0000000000000000
      APPVERSION=01.00
      COMMON=(
        -DBENNUGD_VERSION="${BENNUGD_VERSION}"
        -DBENNUGD_ZLIB_VERSION="${ZLIB_VERSION}"
        -DBENNUGD_LIBPNG_VERSION="${LIBPNG_VERSION}"
        -DBENNUGD_SDL3_REF="${SDL3_REF}"
        -DBENNUGD_SDL3_PS3_REF="${SDL3_PS3_REF}"
        -DBENNUGD_SDL3_MIXER_REF="${SDL3_MIXER_REF}"
      )
      cmake --preset ps3-host "${COMMON[@]}"
      cmake --build --preset ps3-host
      for prg in /src/web/demo/*.prg; do
        dcb="${prg%.prg}.dcb"
        "${HOST_BUILD}/core/bgdc/src/bgdc" -o "${dcb}" "${prg}"
        test -s "${dcb}"
      done
      cmake --preset ps3-ppu \
        "${COMMON[@]}"
      cmake --build --preset ps3-ppu
      ELF=""
      for cand in \
        "${PS3_BUILD}/core/bgdi/src/bgdi.elf" \
        "${PS3_BUILD}/core/bgdi/src/bgdi"
      do
        if [[ -f "${cand}" ]]; then
          ELF="${cand}"
          break
        fi
      done
      if [[ -z "${ELF}" ]]; then
        ELF="$(find "${PS3_BUILD}/core/bgdi" \( -type f -o -type l \) \( -name bgdi.elf -o -name bgdi \) | grep -v /CMakeFiles/ | head -n 1 || true)"
      fi
      test -n "${ELF}"
      test -s "${ELF}"
      rm -rf "${PKGDIR}" "${STAGE}"
      mkdir -p "${PKGDIR}" "${STAGE}"
      cp "${ELF}" "${PKGDIR}/bgdi.elf"
      SPRXLINKER=""
      for cand in "${PS3DEV}/bin/sprxlinker" sprxlinker; do
        if command -v "${cand}" >/dev/null 2>&1 || [[ -x "${cand}" ]]; then
          SPRXLINKER="${cand}"
          break
        fi
      done
      test -n "${SPRXLINKER}"
      "${SPRXLINKER}" "${PKGDIR}/bgdi.elf"
      STRIP=""
      for cand in \
        "${PS3DEV}/ppu/bin/powerpc64-ps3-elf-strip" \
        "${PS3DEV}/bin/ppu-strip" \
        ppu-strip \
        powerpc64-ps3-elf-strip
      do
        if command -v "${cand}" >/dev/null 2>&1 || [[ -x "${cand}" ]]; then
          STRIP="${cand}"
          break
        fi
      done
      if [[ -n "${STRIP}" ]]; then
        "${STRIP}" --strip-debug "${PKGDIR}/bgdi.elf" -o "${PKGDIR}/bgdi.stripped.elf" \
          || "${STRIP}" "${PKGDIR}/bgdi.elf" -o "${PKGDIR}/bgdi.stripped.elf"
      else
        cp "${PKGDIR}/bgdi.elf" "${PKGDIR}/bgdi.stripped.elf"
      fi
      MAKE_SELF_NPDRM=""
      for cand in "${PS3DEV}/bin/make_self_npdrm" make_self_npdrm; do
        if command -v "${cand}" >/dev/null 2>&1 || [[ -x "${cand}" ]]; then
          MAKE_SELF_NPDRM="${cand}"
          break
        fi
      done
      test -n "${MAKE_SELF_NPDRM}"
      mkdir -p "${PKGDIR}/pkg/USRDIR"
      "${MAKE_SELF_NPDRM}" "${PKGDIR}/bgdi.stripped.elf" "${PKGDIR}/pkg/USRDIR/EBOOT.BIN" "${CONTENT_ID}"
      cp /src/web/demo/*.dcb "${PKGDIR}/pkg/USRDIR/"
      cp /src/web/demo/hello.dcb "${PKGDIR}/pkg/USRDIR/main.dcb"
      SFOXML=""
      for cand in /src/ps3/sfo.xml "${PS3DEV}/bin/sfo.xml"; do
        if [[ -f "${cand}" ]]; then
          SFOXML="${cand}"
          break
        fi
      done
      test -n "${SFOXML}"
      cp "${SFOXML}" "${PKGDIR}/sfo.xml"
      sed -i "s/01\\.00/${APPVERSION}/g" "${PKGDIR}/sfo.xml"
      SFO=""
      for cand in "${PS3DEV}/bin/sfo" "${PS3DEV}/bin/sfo.py" sfo sfo.py; do
        if command -v "${cand}" >/dev/null 2>&1 || [[ -x "${cand}" ]]; then
          SFO="${cand}"
          break
        fi
      done
      test -n "${SFO}"
      if [[ "${SFO}" == *.py ]]; then
        python3 "${SFO}" --title "${GAME_TITLE}" --appid "${GAME_ID}" -f "${PKGDIR}/sfo.xml" "${PKGDIR}/pkg/PARAM.SFO"
      else
        "${SFO}" --title "${GAME_TITLE}" --appid "${GAME_ID}" -f "${PKGDIR}/sfo.xml" "${PKGDIR}/pkg/PARAM.SFO"
      fi
      if [[ -f "${PS3DEV}/bin/ICON0.PNG" ]]; then
        cp "${PS3DEV}/bin/ICON0.PNG" "${PKGDIR}/pkg/ICON0.PNG"
      fi
      PKGTOOL=""
      for cand in "${PS3DEV}/bin/pkg" "${PS3DEV}/bin/pkg.py" pkg.py; do
        if command -v "${cand}" >/dev/null 2>&1 || [[ -x "${cand}" ]]; then
          PKGTOOL="${cand}"
          break
        fi
      done
      test -n "${PKGTOOL}"
      if [[ "${PKGTOOL}" == *.py ]]; then
        python3 "${PKGTOOL}" --contentid "${CONTENT_ID}" "${PKGDIR}/pkg/" "${STAGE}/bennugd64.pkg"
      else
        "${PKGTOOL}" --contentid "${CONTENT_ID}" "${PKGDIR}/pkg/" "${STAGE}/bennugd64.pkg"
      fi
      cmake --install "${PS3_BUILD}" --prefix "${STAGE}"
      cp "${PKGDIR}/bgdi.elf" "${PKGDIR}/pkg/USRDIR/EBOOT.BIN" "${PKGDIR}/pkg/USRDIR/main.dcb" "${STAGE}/"
      test -s "${STAGE}/bennugd64.pkg"
      test -s "${STAGE}/EBOOT.BIN"
      test -s "${STAGE}/bgdi.elf"
    '
  exit 0
fi

if [[ "${PLATFORM}" == "pandora" ]]; then
  echo "image: ${IMAGE}"
  echo "preset: pandora-host + pandora-arm"
  echo "version: ${BENNUGD_VERSION}"
  scrub_fetchcontent "${ROOT}/build-pandora-host/_deps"
  scrub_fetchcontent "${ROOT}/build-pandora-arm/_deps"
  mkdir -p "${ROOT}/build-pandora-arm/_deps"
  prefetch_github_archive "${ROOT}/build-pandora-arm/_deps/libpng-src" \
    "https://github.com/pnggroup/libpng/archive/refs/tags/v${LIBPNG_VERSION:-1.6.47}.tar.gz"
  prefetch_github_archive "${ROOT}/build-pandora-arm/_deps/sdl3-src" \
    "https://github.com/libsdl-org/SDL/archive/refs/tags/${SDL3_REF:-release-3.4.14}.tar.gz"
  prefetch_github_archive "${ROOT}/build-pandora-arm/_deps/sdl3_mixer-src" \
    "https://github.com/libsdl-org/SDL_mixer/archive/refs/tags/${SDL3_MIXER_REF:-release-3.2.4}.tar.gz"
  docker run --platform linux/amd64 --rm \
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
      unset CC CXX CFLAGS CXXFLAGS
      test -x /opt/openpandora/bin/arm-angstrom-linux-gnueabi-gcc
      HOST_BUILD=/src/build-pandora-host
      PND_BUILD=/src/build-pandora-arm
      PNDDIR=/src/build-pandora-pnd
      STAGE=/src/dist/pandora-arm-static
      FETCH_DIR="${HOST_BUILD}/_deps"
      COMMON=(
        -DBENNUGD_VERSION="${BENNUGD_VERSION}"
        -DBENNUGD_ZLIB_VERSION="${ZLIB_VERSION}"
        -DBENNUGD_LIBPNG_VERSION="${LIBPNG_VERSION}"
        -DBENNUGD_SDL3_REF="${SDL3_REF}"
        -DBENNUGD_SDL3_MIXER_REF="${SDL3_MIXER_REF}"
      )
      cmake --preset pandora-host "${COMMON[@]}"
      cmake --build --preset pandora-host
      for prg in /src/web/demo/*.prg; do
        dcb="${prg%.prg}.dcb"
        "${HOST_BUILD}/core/bgdc/src/bgdc" -o "${dcb}" "${prg}"
        test -s "${dcb}"
      done
      cmake --preset pandora-arm \
        "${COMMON[@]}" \
        -DFETCHCONTENT_SOURCE_DIR_ZLIB="${FETCH_DIR}/zlib-src" \
        -DFETCHCONTENT_SOURCE_DIR_LIBPNG="${PND_BUILD}/_deps/libpng-src" \
        -DFETCHCONTENT_SOURCE_DIR_SDL3="${PND_BUILD}/_deps/sdl3-src" \
        -DFETCHCONTENT_SOURCE_DIR_SDL3_MIXER="${PND_BUILD}/_deps/sdl3_mixer-src"
      cmake --build --preset pandora-arm
      BGDI="${PND_BUILD}/core/bgdi/src/bgdi"
      test -s "${BGDI}"
      rm -rf "${PNDDIR}"
      mkdir -p "${PNDDIR}" "${STAGE}"
      cp "${BGDI}" "${PNDDIR}/bgdi"
      chmod +x "${PNDDIR}/bgdi"
      cp /src/web/demo/*.dcb "${PNDDIR}/"
      cp /src/web/demo/hello.dcb "${PNDDIR}/main.dcb"
      cp /src/pandora/PXML.xml "${PNDDIR}/PXML.xml"
      PND="${STAGE}/bennugd64.pnd"
      mksquashfs "${PNDDIR}" "${PND}" -all-root -noappend -no-xattrs
      cmake --install "${PND_BUILD}" --prefix "${STAGE}"
      cp "${BGDI}" "${STAGE}/bgdi"
      cp "${PNDDIR}/"* "${STAGE}/" 2>/dev/null || true
      test -s "${STAGE}/bennugd64.pnd"
      test -s "${STAGE}/bgdi"
    '
  exit 0
fi

if [[ "${PLATFORM}" == "wii" ]]; then
  echo "image: ${IMAGE}"
  echo "preset: wii-host + wii-powerpc"
  echo "version: ${BENNUGD_VERSION}"
  scrub_fetchcontent "${ROOT}/build-wii-host/_deps"
  scrub_fetchcontent "${ROOT}/build-wii-powerpc/_deps"
  docker run --platform linux/amd64 --rm \
    -u "$(id -u):$(id -g)" \
    -v "${ROOT}:/src" \
    -w /src \
    -e HOME=/tmp \
    -e BENNUGD_VERSION="${BENNUGD_VERSION}" \
    -e BUILD_TYPE="${BUILD_TYPE:-Release}" \
    -e ZLIB_VERSION="${ZLIB_VERSION:-1.3.1}" \
    -e LIBPNG_VERSION="${LIBPNG_VERSION:-1.6.47}" \
    -e SDL3_REF="${SDL3_REF:-release-3.4.14}" \
    -e SDL3_WII_REF="${SDL3_WII_REF:-fixes}" \
    -e SDL3_MIXER_REF="${SDL3_MIXER_REF:-release-3.2.4}" \
    "${IMAGE}" \
    bash -c 'set -euo pipefail
      unset CC CXX CFLAGS CXXFLAGS
      test -n "${DEVKITPRO:-}"
      test -x "${DEVKITPRO}/devkitPPC/bin/powerpc-eabi-gcc"
      HOST_BUILD=/src/build-wii-host
      WII_BUILD=/src/build-wii-powerpc
      APPDIR=/src/build-wii-app
      STAGE=/src/dist/wii-powerpc-static
      FETCH_DIR="${HOST_BUILD}/_deps"
      COMMON=(
        -DBENNUGD_VERSION="${BENNUGD_VERSION}"
        -DBENNUGD_ZLIB_VERSION="${ZLIB_VERSION}"
        -DBENNUGD_LIBPNG_VERSION="${LIBPNG_VERSION}"
        -DBENNUGD_SDL3_REF="${SDL3_REF}"
        -DBENNUGD_SDL3_WII_REF="${SDL3_WII_REF}"
        -DBENNUGD_SDL3_MIXER_REF="${SDL3_MIXER_REF}"
      )
      cmake --preset wii-host "${COMMON[@]}"
      cmake --build --preset wii-host
      for prg in /src/web/demo/*.prg; do
        dcb="${prg%.prg}.dcb"
        "${HOST_BUILD}/core/bgdc/src/bgdc" -o "${dcb}" "${prg}"
        test -s "${dcb}"
      done
      cmake --preset wii-powerpc \
        "${COMMON[@]}" \
        -DFETCHCONTENT_SOURCE_DIR_ZLIB="${FETCH_DIR}/zlib-src"
      cmake --build --preset wii-powerpc
      ELF=""
      for cand in \
        "${WII_BUILD}/core/bgdi/src/bgdi.elf" \
        "${WII_BUILD}/core/bgdi/src/bgdi"
      do
        if [[ -f "${cand}" ]]; then
          ELF="${cand}"
          break
        fi
      done
      if [[ -z "${ELF}" ]]; then
        ELF="$(find "${WII_BUILD}/core/bgdi" \( -type f -o -type l \) \( -name bgdi.elf -o -name bgdi \) | grep -v /CMakeFiles/ | head -n 1 || true)"
      fi
      test -n "${ELF}"
      test -s "${ELF}"
      ELF2DOL="$(command -v elf2dol || true)"
      if [[ -z "${ELF2DOL}" ]]; then
        for cand in "${DEVKITPRO}/tools/bin/elf2dol"; do
          if [[ -x "${cand}" ]]; then
            ELF2DOL="${cand}"
            break
          fi
        done
      fi
      test -n "${ELF2DOL}"
      rm -rf "${APPDIR}"
      mkdir -p "${APPDIR}" "${STAGE}/apps/bennugd64"
      cp /src/web/demo/*.dcb "${APPDIR}/"
      cp /src/web/demo/hello.dcb "${APPDIR}/main.dcb"
      sed "s|<version>dev</version>|<version>${BENNUGD_VERSION}</version>|" \
        /src/wii/meta.xml > "${APPDIR}/meta.xml"
      DOL="${APPDIR}/boot.dol"
      "${ELF2DOL}" "${ELF}" "${DOL}"
      python3 /src/scripts/pad-wii-dol.py "${DOL}"
      cmake --install "${WII_BUILD}" --prefix "${STAGE}"
      # Debug sections sit at vaddr 0; strip so File→Open of the ELF cannot
      # map them over page 0 in Dolphin.
      if command -v powerpc-eabi-strip >/dev/null; then
        powerpc-eabi-strip -g "${ELF}" || true
      fi
      cp "${ELF}" "${STAGE}/bgdi.elf"
      cp "${APPDIR}/"* "${STAGE}/apps/bennugd64/"
      test -s "${STAGE}/apps/bennugd64/boot.dol"
      test -s "${STAGE}/apps/bennugd64/main.dcb"
      test -s "${STAGE}/bgdi.elf"
    '
  exit 0
fi

if [[ "${PLATFORM}" == "macos" ]]; then
  MACOS_ARCH="${MACOS_ARCH:-x86_64}"
  LINKAGE="static"
  for arg in "${SECOND}" "${3:-}"; do
    [[ -z "${arg}" ]] && continue
    case "${arg}" in
      static|shared) LINKAGE="${arg}" ;;
      arm64|aarch64) MACOS_ARCH="arm64" ;;
      x86_64|amd64) MACOS_ARCH="x86_64" ;;
      *)
        echo "${USAGE}" >&2
        exit 1
        ;;
    esac
  done
  PRESET="macos-${MACOS_ARCH}-${LINKAGE}"
  BUILD_DIR="/src/build-macos-${MACOS_ARCH}-${LINKAGE}"
  STAGE="/src/dist/macos-${MACOS_ARCH}-${LINKAGE}"
  echo "image: ${IMAGE}"
  echo "preset: ${PRESET}"
  echo "version: ${BENNUGD_VERSION}"
  # Failed configure leaves Darwin binutils unset in the cache.
  rm -f "${ROOT}/${BUILD_DIR#/src/}/CMakeCache.txt"
  rm -rf "${ROOT}/${BUILD_DIR#/src/}/CMakeFiles"
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
      if command -v osxcross-conf >/dev/null; then
        eval "$(osxcross-conf)"
      fi
      cmake --preset "${PRESET}" \
        -DBENNUGD_VERSION="${BENNUGD_VERSION}" \
        -DBENNUGD_ZLIB_VERSION="${ZLIB_VERSION}" \
        -DBENNUGD_LIBPNG_VERSION="${LIBPNG_VERSION}" \
        -DBENNUGD_SDL3_REF="${SDL3_REF}" \
        -DBENNUGD_SDL3_MIXER_REF="${SDL3_MIXER_REF}"
      cmake --build --preset "${PRESET}"
      cmake --install "${BUILD_DIR}" --prefix "${STAGE}"
      bash /src/scripts/osx/codesign.sh "${STAGE}"
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
