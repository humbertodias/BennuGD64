#!/usr/bin/env bash
# Generate Doxygen HTML under doc/html/ (see doc/pages.yml).
# Prefers the bennugd64-doxygen Docker image; falls back to a local doxygen binary.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
IMAGE="${BENNUGD_DOXYGEN_IMAGE:-bennugd64-doxygen}"

run_inner() {
  bash "$ROOT/doc/generate-inner.sh"
}

if [[ "${USE_DOCKER:-}" != "1" ]] && command -v doxygen >/dev/null 2>&1; then
  run_inner
  exit 0
fi

if ! command -v docker >/dev/null 2>&1; then
  echo "Docker is required when doxygen is not installed (https://docs.docker.com/get-docker/)." >&2
  echo "Or install doxygen locally, or set USE_DOCKER=0 to skip the container." >&2
  exit 1
fi

if [[ "${SKIP_DOCKER_BUILD:-}" != "1" ]]; then
  docker build -t "$IMAGE" -f "$ROOT/docker/Dockerfile.doxygen" "$ROOT/docker"
fi

docker run --rm \
  -v "$ROOT:/src" \
  -w /src/doc \
  "$IMAGE" \
  bash /src/doc/generate-inner.sh
