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

mkdir -p "$DST/docs" "$DST/ide" "$DST/samples"
cp "$WASM_SRC/index.html" "$DST/"
cp "$WASM_SRC/bgdi.js" "$WASM_SRC/bgdi.wasm" "$WASM_SRC/bgdi.data" "$DST/"
cp "$WASM_SRC/bgdi.wasm.map" "$DST/" 2>/dev/null || true
cp "$WASM_SRC/bgdi.js.map" "$DST/" 2>/dev/null || true
if [[ -f "$WASM_SRC/bgdc.wasm" ]]; then
  cp "$WASM_SRC/bgdc.wasm" "$DST/"
fi
if [[ -d "$WASM_SRC/ide" ]]; then
  cp -a "$WASM_SRC/ide/." "$DST/ide/"
else
  cp -a "$ROOT/web/ide/." "$DST/ide/"
fi
if [[ -d "$WASM_SRC/samples" ]]; then
  cp -a "$WASM_SRC/samples/." "$DST/samples/"
else
  cp "$ROOT/web/demo/"*.prg "$DST/samples/"
fi

cp -a "$ROOT/doc/html/." "$DST/docs/"
touch "$DST/.nojekyll"

test -s "$DST/index.html"
test -s "$DST/bgdi.js"
test -s "$DST/bgdi.wasm"
test -s "$DST/bgdi.data"
test -s "$DST/ide/index.html"
test -s "$DST/samples/hello.prg"
test -s "$DST/docs/index.html"
if [[ -f "$WASM_SRC/bgdc.wasm" ]]; then
  test -s "$DST/bgdc.wasm"
fi

echo "Pages site staged at: $DST ($(find "$DST" -type f | wc -l) files)"
