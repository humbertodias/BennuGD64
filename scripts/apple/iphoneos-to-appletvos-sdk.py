#!/usr/bin/env python3
"""Copy an iPhoneOS.sdk tree and retarget TBD/SDKSettings at tvOS for ld64."""
import os
import re
import shutil
import sys

REPLACEMENTS = (
    ("arm64e-ios", "arm64e-tvos"),
    ("arm64-ios", "arm64-tvos"),
    ("armv7s-ios", "armv7s-tvos"),
    ("armv7-ios", "armv7-tvos"),
    ("x86_64-ios", "x86_64-tvos"),
    ("i386-ios", "i386-tvos"),
)

TEXT_SUFFIXES = (".tbd", ".json", ".plist", ".modulemap", ".map")


def patch_text(data: str) -> str:
    for old, new in REPLACEMENTS:
        data = data.replace(old, new)
    data = data.replace("iphoneos", "appletvos")
    data = data.replace("iPhoneOS", "AppleTVOS")
    data = data.replace("iPhone OS", "tvOS")
    return data


def patch_metal_featuresets(sdk: str) -> None:
    """iPhoneOS Metal.h lists tvOS GPUFamily1 but not GPUFamily2 (Apple TV 4K)."""
    path = os.path.join(
        sdk, "System/Library/Frameworks/Metal.framework/Headers/MTLDevice.h"
    )
    if not os.path.isfile(path):
        return
    with open(path, encoding="utf-8") as fh:
        text = fh.read()
    if "MTLFeatureSet_tvOS_GPUFamily2_v1" not in text:
        text, n = re.subn(
            r"(MTLFeatureSet_tvOS_GPUFamily1_v3 API_AVAILABLE\(tvos\(11\.0\)\)"
            r" API_UNAVAILABLE\(macos, ios\) = 30002,)",
            r"\1\n    MTLFeatureSet_tvOS_GPUFamily2_v1 "
            r"API_AVAILABLE(tvos(11.0)) API_UNAVAILABLE(macos, ios) = 30003,",
            text,
            count=1,
        )
        if n != 1:
            print(f"warning: no GPUFamily1_v3 insertion point in {path}", file=sys.stderr)
    if "MTLFeatureSet_tvOS_GPUFamily2_v2" not in text:
        text, n = re.subn(
            r"(MTLFeatureSet_tvOS_GPUFamily1_v4 API_AVAILABLE\(tvos\(12\.0\)\)"
            r" API_UNAVAILABLE\(macos, ios\) = 30004,)",
            r"\1\n    MTLFeatureSet_tvOS_GPUFamily2_v2 "
            r"API_AVAILABLE(tvos(12.0)) API_UNAVAILABLE(macos, ios) = 30005,",
            text,
            count=1,
        )
        if n != 1:
            print(f"warning: no GPUFamily1_v4 insertion point in {path}", file=sys.stderr)
    with open(path, "w", encoding="utf-8") as fh:
        fh.write(text)


def main() -> int:
    if len(sys.argv) != 3:
        print(f"usage: {sys.argv[0]} src-iphoneos.sdk dest-appletvos.sdk", file=sys.stderr)
        return 2
    src, dst = sys.argv[1], sys.argv[2]
    if not os.path.isdir(src):
        print(f"missing {src}", file=sys.stderr)
        return 1
    if os.path.abspath(src) != os.path.abspath(dst):
        if os.path.exists(dst):
            shutil.rmtree(dst)
        shutil.copytree(src, dst, symlinks=True)
    for root, _dirs, files in os.walk(dst):
        for name in files:
            path = os.path.join(root, name)
            if not name.endswith(TEXT_SUFFIXES):
                continue
            try:
                with open(path, "r", encoding="utf-8") as fh:
                    text = fh.read()
            except (UnicodeDecodeError, OSError):
                continue
            patched = patch_text(text)
            if patched != text:
                with open(path, "w", encoding="utf-8") as fh:
                    fh.write(patched)
    patch_metal_featuresets(dst)
    print(f"AppleTVOS SDK -> {dst}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
