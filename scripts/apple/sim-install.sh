#!/usr/bin/env bash
# Linux zip + Finder unzip drop +x; osxcross also omitted the ad-hoc Mach-O
# signature. Simulator then launches bgdi.app and kills it immediately.
# Run this on a Mac from the extracted folder (or pass the .app path).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
APP="${1:-}"
if [[ -z "${APP}" ]]; then
  if [[ -d "${ROOT}/bgdi.app" ]]; then
    APP="${ROOT}/bgdi.app"
  else
    APP="$(find "${ROOT}" -name bgdi.app -type d | grep -v /CMakeFiles/ | head -n 1 || true)"
  fi
fi
if [[ -z "${APP}" || ! -d "${APP}" ]]; then
  echo "usage: $0 [bgdi.app]" >&2
  exit 1
fi
APP="$(cd "${APP}" && pwd)"
test -f "${APP}/bgdi"

chmod +x "${APP}/bgdi"
if ! command -v codesign >/dev/null 2>&1; then
  echo "codesign not found (need a Mac with Xcode CLT)" >&2
  exit 1
fi
codesign --force --sign - --timestamp=none "${APP}/bgdi"
codesign --force --sign - --timestamp=none "${APP}"
echo "adhoc-signed ${APP}"

if command -v xcrun >/dev/null 2>&1 && xcrun simctl list devices booted 2>/dev/null | grep -q Booted; then
  xcrun simctl install booted "${APP}"
  echo "installed on the booted Simulator"
else
  echo "boot a Simulator, then: xcrun simctl install booted \"${APP}\""
fi
