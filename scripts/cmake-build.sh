#!/usr/bin/env bash
# Configure, build, install and smoke-test BennuGD64.
# Used by docker/Dockerfile.{linux,windows} (via scripts/docker-build.sh) and native CI.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
if [[ -f "${ROOT}/versions.env" ]]; then
  set -a
  # shellcheck disable=SC1091
  source "${ROOT}/versions.env"
  set +a
fi

PLATFORM="${PLATFORM:-linux}"
LINKAGE="${LINKAGE:-static}"
SRC_DIR="${SRC_DIR:-${ROOT}}"
BUILD_TYPE="${BUILD_TYPE:-Release}"

if [[ -z "${BUILD_DIR:-}" ]]; then
  BUILD_DIR="${ROOT}/build-${PLATFORM}-${LINKAGE}"
fi
if [[ -z "${STAGE:-}" ]]; then
  case "${PLATFORM}" in
    linux) STAGE="${ROOT}/dist/linux-${LINKAGE}" ;;
    windows) STAGE="${ROOT}/dist/windows-x86_64-${LINKAGE}" ;;
    macos) STAGE="${ROOT}/dist/macos-arm64-${LINKAGE}" ;;
    wasi) STAGE="${ROOT}/dist/wasi-wasm32-static" ;;
    *) STAGE="${ROOT}/dist/${PLATFORM}-${LINKAGE}" ;;
  esac
fi

STATIC_MODULES=ON
if [[ "${LINKAGE}" != "static" ]]; then
  STATIC_MODULES=OFF
fi

CMAKE_ARGS=(
  -S "${SRC_DIR}"
  -B "${BUILD_DIR}"
  -G Ninja
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
  -DUSE_LIBDES=ON
  -DBENNUGD_BUNDLE_DEPS=ON
  -DSTATIC_MODULES="${STATIC_MODULES}"
  -DNO_SOUND=OFF
  -DBENNUGD_ZLIB_VERSION="${BENNUGD_ZLIB_VERSION:-${ZLIB_VERSION:-1.3.1}}"
  -DBENNUGD_LIBPNG_VERSION="${BENNUGD_LIBPNG_VERSION:-${LIBPNG_VERSION:-1.6.47}}"
  -DBENNUGD_SDL3_REF="${BENNUGD_SDL3_REF:-${SDL3_REF:-release-3.4.14}}"
  -DBENNUGD_SDL3_MIXER_REF="${BENNUGD_SDL3_MIXER_REF:-${SDL3_MIXER_REF:-release-3.2.4}}"
)

if [[ -n "${BENNUGD_VERSION:-}" ]]; then
  CMAKE_ARGS+=(-DBENNUGD_VERSION="${BENNUGD_VERSION}")
fi
if [[ -n "${CMAKE_TOOLCHAIN_FILE:-}" ]]; then
  CMAKE_ARGS+=(-DCMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAIN_FILE}")
fi

BUILD_TARGET=""
if [[ "${PLATFORM}" == "wasi" ]]; then
  CMAKE_ARGS+=(
    -DBENNUGD_WASI=ON
    -DCOMPILER_ONLY=ON
    -DSTATIC_MODULES=ON
    -DBENNUGD_WASI_SDK_VERSION="${WASI_SDK_VERSION:-33}"
    -DBENNUGD_WASI_SDK_VERSION_FULL="${WASI_SDK_VERSION_FULL:-33.0}"
  )
  BUILD_TARGET="bgdc"
fi

echo "toolchain: $(${CC:-cc} --version | head -1 || true)"
command -v cmake
command -v ninja
cmake "${CMAKE_ARGS[@]}"

if [[ -n "${BUILD_TARGET}" ]]; then
  cmake --build "${BUILD_DIR}" --config "${BUILD_TYPE}" --target "${BUILD_TARGET}"
else
  cmake --build "${BUILD_DIR}" --config "${BUILD_TYPE}"
fi

cmake --install "${BUILD_DIR}" --prefix "${STAGE}" --config "${BUILD_TYPE}"

# Compile web/demo/hello.prg with the staged compiler. hello.prg never exits
# without ESC, so we only check that a non-empty .dcb is produced.
# Usage: compile_hello_dcb [--copy FILE]... command [args...]
compile_hello_dcb() {
  local smoke
  smoke="$(mktemp -d)"
  cp "${SRC_DIR}/web/demo/hello.prg" "${smoke}/hello.prg"
  while [[ "${1:-}" == "--copy" ]]; do
    shift
    cp "$1" "${smoke}/"
    shift
  done
  echo "----- compile hello.prg -----"
  if (
    cd "${smoke}"
    "$@"
    test -s hello.dcb
    ls -l hello.dcb
  ); then
    rm -rf "${smoke}"
    return 0
  fi
  rm -rf "${smoke}"
  return 1
}

copy_mingw_runtime() {
  local dest="$1"
  local dll gcc_dll
  gcc_dll="$(x86_64-w64-mingw32-gcc -print-file-name=libgcc_s_seh-1.dll 2>/dev/null || true)"
  for dll in libgcc_s_seh-1.dll libwinpthread-1.dll libstdc++-6.dll; do
    if [[ -f "${dest}/${dll}" ]]; then
      continue
    fi
    if [[ "${dll}" == "libgcc_s_seh-1.dll" && -f "${gcc_dll}" && "${gcc_dll}" != "libgcc_s_seh-1.dll" ]]; then
      cp "${gcc_dll}" "${dest}/"
      continue
    fi
    if [[ -f "/usr/x86_64-w64-mingw32/lib/${dll}" ]]; then
      cp "/usr/x86_64-w64-mingw32/lib/${dll}" "${dest}/"
    elif [[ -f "/usr/x86_64-w64-mingw32/bin/${dll}" ]]; then
      cp "/usr/x86_64-w64-mingw32/bin/${dll}" "${dest}/"
    fi
  done
}

smoke_unix() {
  local bgdc="${STAGE}/bgdc"
  local bgdi="${STAGE}/bgdi"
  echo "bgdc=${bgdc}"
  echo "bgdi=${bgdi}"
  ls -la "${STAGE}"
  "${bgdc}" >/tmp/bgdc-help.txt 2>&1 || true
  "${bgdi}" >/tmp/bgdi-help.txt 2>&1 || true
  echo "----- bgdc output -----"
  cat /tmp/bgdc-help.txt || true
  echo "----- bgdi output -----"
  cat /tmp/bgdi-help.txt || true
  grep -E 'BGDC|Compiler' /tmp/bgdc-help.txt
  grep -E 'BGDI|Interpreter' /tmp/bgdi-help.txt
  echo "Dynamic libs (informational):"
  (otool -L "${bgdi}" 2>/dev/null || ldd "${bgdi}" || true) | head -40
  compile_hello_dcb "${bgdc}" -o hello.dcb hello.prg
}

smoke_windows() {
  copy_mingw_runtime "${STAGE}"
  local bgdc="${STAGE}/bgdc.exe"
  local bgdi="${STAGE}/bgdi.exe"
  test -f "${bgdc}"
  test -f "${bgdi}"
  echo "bgdc=${bgdc}"
  echo "bgdi=${bgdi}"
  ls -la "${STAGE}"
  echo "DLL dependencies (informational):"
  x86_64-w64-mingw32-objdump -p "${bgdi}" | grep -i 'DLL Name' | head -40 || true
}

smoke_wasi() {
  local wasm="${STAGE}/bgdc.wasm"
  test -s "${wasm}"
  ls -lah "${STAGE}"
  wasmtime --dir=. "${wasm}" -- >/tmp/bgdc-wasi-help.txt 2>&1 || true
  echo "----- bgdc.wasm output -----"
  cat /tmp/bgdc-wasi-help.txt
  grep -E 'BGDC|Compiler' /tmp/bgdc-wasi-help.txt
  compile_hello_dcb --copy "${wasm}" wasmtime --dir=. ./bgdc.wasm -- -o hello.dcb hello.prg
}

case "${PLATFORM}" in
  linux|macos) smoke_unix ;;
  windows) smoke_windows ;;
  wasi) smoke_wasi ;;
  *)
    echo "Unknown PLATFORM=${PLATFORM}" >&2
    exit 1
    ;;
esac
