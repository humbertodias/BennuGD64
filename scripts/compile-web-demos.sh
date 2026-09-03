#!/usr/bin/env bash
# Compile platforms/web/demo/*.prg → .dcb only when the .dcb is missing or
# older than its .prg (avoids dirtying tracked .dcb files on every build).
#
#   scripts/compile-web-demos.sh /path/to/bgdc [/path/to/demo]
set -euo pipefail

bgdc="${1:?usage: $0 /path/to/bgdc [/path/to/demo]}"
if [[ -n "${2:-}" ]]; then
  demo_dir="$2"
else
  root="$(cd "$(dirname "$0")/.." && pwd)"
  demo_dir="${root}/platforms/web/demo"
fi

if [[ ! -f "${bgdc}" ]]; then
  echo "compile-web-demos: bgdc not found: ${bgdc}" >&2
  exit 1
fi
if [[ ! -d "${demo_dir}" ]]; then
  echo "compile-web-demos: demo dir not found: ${demo_dir}" >&2
  exit 1
fi

shopt -s nullglob
compiled=0
skipped=0
for prg in "${demo_dir}"/*.prg; do
  dcb="${prg%.prg}.dcb"
  if [[ -s "${dcb}" && ! "${prg}" -nt "${dcb}" ]]; then
    skipped=$((skipped + 1))
    continue
  fi
  echo "compile-web-demos: ${prg##*/} → ${dcb##*/}"
  "${bgdc}" -o "${dcb}" "${prg}"
  test -s "${dcb}"
  compiled=$((compiled + 1))
done

if [[ "${compiled}" -eq 0 && "${skipped}" -eq 0 ]]; then
  echo "compile-web-demos: no .prg files in ${demo_dir}" >&2
  exit 1
fi
echo "compile-web-demos: compiled ${compiled}, skipped ${skipped}"
