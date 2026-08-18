#!/usr/bin/env bash
# Stage dist/pages from wasm CI artifact + Doxygen (see doc/pages.yml).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
WASM_SRC="${1:-}"
DST="${2:-$ROOT/dist/pages}"

if [[ -z "$WASM_SRC" ]]; then
  echo "usage: doc/stage-site.sh <web-wasm32-static-dir> [output-dir]" >&2
  exit 1
fi

if [[ ! -d "$WASM_SRC" ]]; then
  echo "wasm source directory not found: $WASM_SRC" >&2
  exit 1
fi

bash "$ROOT/doc/generate.sh"

mkdir -p "$DST/docs"
cp "$WASM_SRC/index.html" "$DST/"
cp "$WASM_SRC/bgdi.js" "$WASM_SRC/bgdi.wasm" "$WASM_SRC/bgdi.data" "$DST/"
cp "$WASM_SRC/bgdi.wasm.map" "$DST/" 2>/dev/null || true
cp "$WASM_SRC/bgdi.js.map" "$DST/" 2>/dev/null || true

cp -a "$ROOT/doc/html/." "$DST/docs/"
touch "$DST/.nojekyll"

test -s "$DST/index.html"
test -s "$DST/bgdi.js"
test -s "$DST/bgdi.wasm"
test -s "$DST/bgdi.data"
test -s "$DST/docs/index.html"

echo "Pages site staged at: $DST"
ls -lah "$DST" "$DST/docs" | head -40
