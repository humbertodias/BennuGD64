#!/usr/bin/env bash
# Ad-hoc sign staged macOS binaries. Apple's install_name_tool re-signs;
# osxcross's does not. bgdi/bgdc get disable-library-validation so @rpath
# dylibs can load after a download (Gatekeeper library policy).
set -euo pipefail

STAGE="${1:?usage: $0 <install-prefix>}"
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
ENTITLEMENTS="${ROOT}/scripts/macos/macos-disable-library-validation.plist"

if command -v osxcross-codesign >/dev/null 2>&1; then
  sign_lib() { osxcross-codesign -s - -f "$1"; }
  sign_bin() { osxcross-codesign -s - -f --entitlements "${ENTITLEMENTS}" "$1"; }
elif command -v codesign >/dev/null 2>&1; then
  sign_lib() { codesign --force --sign - --timestamp=none "$1"; }
  sign_bin() {
    codesign --force --sign - --timestamp=none --entitlements "${ENTITLEMENTS}" "$1"
  }
else
  echo "codesign: no codesign/osxcross-codesign" >&2
  exit 1
fi

is_macho() {
  local magic
  magic="$(od -An -tx4 -N4 "$1" 2>/dev/null | tr -d ' \n')"
  case "${magic}" in
    feedfacf|cffaedfe|cafebabe|bebafeca) return 0 ;;
    *) return 1 ;;
  esac
}

shopt -s nullglob
for f in "${STAGE}"/*.dylib "${STAGE}/modules"/*.dylib; do
  [[ -f "${f}" ]] || continue
  is_macho "${f}" || continue
  echo "codesign: ${f}"
  sign_lib "${f}"
done
for f in "${STAGE}/bgdi" "${STAGE}/bgdc"; do
  [[ -f "${f}" ]] || continue
  is_macho "${f}" || continue
  echo "codesign: ${f}"
  sign_bin "${f}"
done
