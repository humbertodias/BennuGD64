#!/usr/bin/env bash
# Build artifacts in a toolchain image (CMake presets + CTest).
#
#   bash scripts/docker-build.sh
#   bash scripts/docker-build.sh linux shared
#   bash scripts/docker-build.sh windows
#   bash scripts/docker-build.sh wasm
#   bash scripts/docker-build.sh android
#   bash scripts/docker-build.sh switch
#   bash scripts/docker-build.sh dreamcast
#   bash scripts/docker-build.sh pandora
#   bash scripts/docker-build.sh linux shell
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "${ROOT}"

USAGE="usage: $0 linux|windows [static|shared|shell]
       $0 wasm [shell]
       $0 android [shell]
       $0 switch [shell]
       $0 dreamcast [shell]
       $0 pandora [shell]"

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
  linux|windows|wasm|android|switch|dreamcast|pandora) ;;
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
  elif [[ "${PLATFORM}" == "pandora" ]]; then
    docker build \
      --platform linux/amd64 \
      -t bennugd64-pandora \
      -f docker/Dockerfile.pandora \
      docker/
  else
    docker build -t "${IMAGE}" -f "docker/Dockerfile.${PLATFORM}" docker/
  fi
fi

if [[ "${SECOND}" == "shell" ]]; then
  if [[ "${PLATFORM}" == "android" || "${PLATFORM}" == "switch" || "${PLATFORM}" == "dreamcast" || "${PLATFORM}" == "pandora" ]]; then
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
    "${ROOT}/build-android-arm64/_deps/${name}" \
    "${ROOT}/build-host/_deps/${name}"
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
      emcmake cmake -S /src -B "${WASM_BUILD}" -G Ninja \
        -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -DUSE_LIBDES=ON \
        -DBENNUGD_BUNDLE_DEPS=ON \
        -DSTATIC_MODULES=ON \
        -DINTERPRETER_ONLY=ON \
        "${COMMON[@]}" \
        -DFETCHCONTENT_BASE_DIR="${WASM_BUILD}/_deps" \
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
