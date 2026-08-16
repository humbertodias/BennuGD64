#!/usr/bin/env bash
# Build Linux or Windows artifacts with a toolchain image. No local compiler required.
#
#   bash scripts/docker-build.sh
#   bash scripts/docker-build.sh linux shared
#   bash scripts/docker-build.sh windows
#   bash scripts/docker-build.sh windows shared
#   bash scripts/docker-build.sh linux shell
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "${ROOT}"

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
  linux|windows) ;;
  *)
    echo "usage: $0 linux|windows [static|shared|shell]" >&2
    exit 1
    ;;
esac

IMAGE="bennugd64-${PLATFORM}"
DOCKERFILE="docker/Dockerfile.${PLATFORM}"

if [[ "${SKIP_DOCKER_BUILD:-}" != "1" ]]; then
  docker build -t "${IMAGE}" -f "${DOCKERFILE}" docker/
fi

if [[ "${SECOND}" == "shell" ]]; then
  exec docker run --rm -it \
    -v "${ROOT}:/src" \
    -w /src \
    -e HOME=/tmp \
    "${IMAGE}" bash
fi

LINKAGE="${SECOND}"
case "${LINKAGE}" in
  static|shared) ;;
  *)
    echo "usage: $0 linux|windows [static|shared|shell]" >&2
    exit 1
    ;;
esac

if [[ -z "${BENNUGD_VERSION:-}" ]]; then
  BENNUGD_VERSION="$(git describe --tags --exact-match HEAD 2>/dev/null \
    || git describe --tags --always --dirty 2>/dev/null \
    || echo dev)"
fi

if [[ "${PLATFORM}" == "windows" ]]; then
  STAGE="/src/dist/windows-x86_64-${LINKAGE}"
else
  STAGE="/src/dist/linux-${LINKAGE}"
fi

echo "image: ${IMAGE}"
echo "linkage: ${LINKAGE}"
echo "version: ${BENNUGD_VERSION}"

docker run --rm \
  -u "$(id -u):$(id -g)" \
  -v "${ROOT}:/src" \
  -w /src \
  -e HOME=/tmp \
  -e PLATFORM="${PLATFORM}" \
  -e LINKAGE="${LINKAGE}" \
  -e SRC_DIR=/src \
  -e BUILD_DIR="/src/build-${PLATFORM}-${LINKAGE}" \
  -e STAGE="${STAGE}" \
  -e BENNUGD_VERSION="${BENNUGD_VERSION}" \
  -e BUILD_TYPE="${BUILD_TYPE:-Release}" \
  -e ZLIB_VERSION="${ZLIB_VERSION:-1.3.1}" \
  -e LIBPNG_VERSION="${LIBPNG_VERSION:-1.6.47}" \
  -e SDL3_REF="${SDL3_REF:-release-3.4.14}" \
  -e SDL3_MIXER_REF="${SDL3_MIXER_REF:-release-3.2.4}" \
  "${IMAGE}" \
  bash scripts/cmake-build.sh
