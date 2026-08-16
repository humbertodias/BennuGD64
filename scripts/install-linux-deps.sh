#!/usr/bin/env bash
# Install docker/apt-linux.txt on a Debian/Ubuntu host (CI wasm jobs).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
sudo apt-get update
grep -vE '^\s*(#|$)' "${ROOT}/docker/apt-linux.txt" \
  | sudo xargs apt-get install -y --no-install-recommends
