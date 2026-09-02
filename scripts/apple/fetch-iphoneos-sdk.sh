#!/bin/sh
# Sparse-checkout one iPhoneOS.sdk from xybp888/iOS-SDKs (headers + TBD stubs).
# Apple's SDK license: https://www.apple.com/legal/sla/docs/xcode.pdf
set -eu
DEST="${1:-/opt/apple/iPhoneOS.sdk}"
SDK_NAME="${IPHONEOS_SDK_NAME:-iPhoneOS17.5.sdk}"
REPO="${IPHONEOS_SDK_REPO:-https://github.com/xybp888/iOS-SDKs.git}"
REF="${IPHONEOS_SDK_REF:-1b92ff4a8928f582876e1d388d1381c6a0c59eb9}"

tmp="$(mktemp -d)"
trap 'rm -rf "${tmp}"' EXIT
git clone --filter=blob:none --no-checkout "${REPO}" "${tmp}/sdk"
git -C "${tmp}/sdk" sparse-checkout init --cone
git -C "${tmp}/sdk" sparse-checkout set "${SDK_NAME}"
git -C "${tmp}/sdk" fetch --depth 1 origin "${REF}"
git -C "${tmp}/sdk" checkout "${REF}"
if [ ! -d "${tmp}/sdk/${SDK_NAME}" ]; then
  echo "missing ${SDK_NAME} at ${REF}" >&2
  exit 1
fi
mkdir -p "$(dirname "${DEST}")"
rm -rf "${DEST}"
mv "${tmp}/sdk/${SDK_NAME}" "${DEST}"
if [ ! -e "${DEST}/SDKSettings.plist" ] && [ ! -e "${DEST}/SDKSettings.json" ]; then
  echo "not an SDK: ${DEST}" >&2
  exit 1
fi
echo "iPhoneOS SDK -> ${DEST}"
