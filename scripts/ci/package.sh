#!/usr/bin/env bash
# Collect bgdc/bgdi into a platform archive under dist/.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build-ci}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
DIST_DIR="${DIST_DIR:-$ROOT/dist}"
PLATFORM="${PLATFORM:-$(uname -s | tr '[:upper:]' '[:lower:]')}"
ARCH="${ARCH:-$(uname -m)}"
VERSION="${VERSION:-dev}"

case "$PLATFORM" in
  darwin) PLATFORM=macos ;;
  msys*|mingw*|cygwin*|windows) PLATFORM=windows ;;
  linux) PLATFORM=linux ;;
esac

STAGE="$DIST_DIR/stage/bennugd64-${VERSION}-${PLATFORM}-${ARCH}"
rm -rf "$STAGE"
mkdir -p "$STAGE"

find_bin() {
  local name="$1"
  local path
  for path in \
    "$BUILD_DIR/core/$name/src/$name" \
    "$BUILD_DIR/core/$name/src/$name.exe" \
    "$BUILD_DIR/core/$name/src/${BUILD_TYPE}/$name.exe" \
    "$BUILD_DIR/core/$name/src/${BUILD_TYPE}/$name"
  do
    if [[ -f "$path" ]]; then
      echo "$path"
      return 0
    fi
  done
  path="$(find "$BUILD_DIR" -type f \( -name "$name" -o -name "$name.exe" \) | head -1 || true)"
  if [[ -n "${path:-}" && -f "$path" ]]; then
    echo "$path"
    return 0
  fi
  echo "Could not find binary: $name" >&2
  find "$BUILD_DIR" -type f \( -name 'bgd*' -o -name 'bgd*.exe' \) 2>/dev/null || true
  return 1
}

BGDC="$(find_bin bgdc)"
BGDI="$(find_bin bgdi)"

cp "$BGDC" "$STAGE/"
cp "$BGDI" "$STAGE/"
if [[ "$PLATFORM" == "windows" ]]; then
  [[ -f "$STAGE/bgdc.exe" ]] || mv "$STAGE/bgdc" "$STAGE/bgdc.exe" 2>/dev/null || true
  [[ -f "$STAGE/bgdi.exe" ]] || mv "$STAGE/bgdi" "$STAGE/bgdi.exe" 2>/dev/null || true
fi

cp "$ROOT/README.md" "$STAGE/" 2>/dev/null || true
cat > "$STAGE/BUILD_INFO.txt" <<EOF
BennuGD64 ${VERSION}
platform: ${PLATFORM}
arch: ${ARCH}
static deps: zlib, libpng, SDL3, SDL3_mixer (stb_vorbis + dr_mp3), bundled DES
note: OS graphics/audio system libraries may still be required at runtime (X11/Wayland/Cocoa/DirectX).
EOF

mkdir -p "$DIST_DIR"
ARCHIVE_BASE="$DIST_DIR/bennugd64-${VERSION}-${PLATFORM}-${ARCH}"
rm -f "${ARCHIVE_BASE}.zip" "${ARCHIVE_BASE}.tar.gz"

STAGE_PARENT="$DIST_DIR/stage"
STAGE_NAME="$(basename "$STAGE")"

make_zip() {
  local out="$1"
  local parent="$2"
  local name="$3"
  # Info-ZIP under MSYS2 has produced truncated archives with absolute Windows
  # paths; use Python's zipfile for a portable, seekable central directory.
  python3 - "$out" "$parent" "$name" <<'PY'
import sys, zipfile
from pathlib import Path

out, parent, name = Path(sys.argv[1]), Path(sys.argv[2]), sys.argv[3]
root = parent / name
with zipfile.ZipFile(out, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=6) as zf:
    for path in sorted(root.rglob("*")):
        if path.is_file():
            zf.write(path, path.relative_to(parent).as_posix())
PY
  python3 - "$out" <<'PY'
import sys, zipfile
from pathlib import Path
out = Path(sys.argv[1])
with zipfile.ZipFile(out) as zf:
    bad = zf.testzip()
    if bad is not None:
        raise SystemExit(f"corrupt zip entry: {bad}")
    print(f"    zip ok: {out} ({out.stat().st_size} bytes, {len(zf.namelist())} entries)")
PY
}

if [[ "$PLATFORM" == "windows" ]]; then
  make_zip "${ARCHIVE_BASE}.zip" "$STAGE_PARENT" "$STAGE_NAME"
  echo "Wrote ${ARCHIVE_BASE}.zip"
else
  tar -C "$STAGE_PARENT" -czf "${ARCHIVE_BASE}.tar.gz" "$STAGE_NAME"
  echo "Wrote ${ARCHIVE_BASE}.tar.gz"
fi

mkdir -p "$DIST_DIR/binaries"
cp -f "$STAGE"/bgdc* "$DIST_DIR/binaries/" 2>/dev/null || true
cp -f "$STAGE"/bgdi* "$DIST_DIR/binaries/" 2>/dev/null || true
