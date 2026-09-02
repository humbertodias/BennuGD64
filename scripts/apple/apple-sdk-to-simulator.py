#!/usr/bin/env python3
"""Copy a device iPhoneOS/AppleTVOS.sdk and retarget TBD/SDKSettings at Simulator."""
import os
import shutil
import sys

IOS_TRIPLES = (
    "arm64e-ios",
    "arm64-ios",
    "armv7s-ios",
    "armv7-ios",
    "x86_64-ios",
    "i386-ios",
)
TVOS_TRIPLES = (
    "arm64e-tvos",
    "arm64-tvos",
    "armv7s-tvos",
    "armv7-tvos",
    "x86_64-tvos",
    "i386-tvos",
)
TEXT_SUFFIXES = (".tbd", ".json", ".plist", ".modulemap", ".map")


def retarget_triples(data: str, triples: tuple) -> str:
    for triple in triples:
        sim = f"{triple}-simulator"
        data = data.replace(sim, "\0SIM\0")
        data = data.replace(triple, sim)
        data = data.replace("\0SIM\0", sim)
    return data


def patch_ios(data: str) -> str:
    data = retarget_triples(data, IOS_TRIPLES)
    data = data.replace("iPhoneSimulator", "\0NAME\0")
    data = data.replace("iphonesimulator", "\0id\0")
    data = data.replace("iPhoneOS", "iPhoneSimulator")
    data = data.replace("iphoneos", "iphonesimulator")
    data = data.replace("\0NAME\0", "iPhoneSimulator")
    data = data.replace("\0id\0", "iphonesimulator")
    return data


def patch_tvos(data: str) -> str:
    data = retarget_triples(data, TVOS_TRIPLES)
    data = data.replace("AppleTVSimulator", "\0NAME\0")
    data = data.replace("appletvsimulator", "\0id\0")
    data = data.replace("AppleTVOS", "AppleTVSimulator")
    data = data.replace("appletvos", "appletvsimulator")
    data = data.replace("\0NAME\0", "AppleTVSimulator")
    data = data.replace("\0id\0", "appletvsimulator")
    return data


def main() -> int:
    if len(sys.argv) != 4 or sys.argv[3] not in ("ios", "tvos"):
        print(
            f"usage: {sys.argv[0]} src-device.sdk dest-simulator.sdk ios|tvos",
            file=sys.stderr,
        )
        return 2
    src, dst, kind = sys.argv[1], sys.argv[2], sys.argv[3]
    patch = patch_ios if kind == "ios" else patch_tvos
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
                with open(path, encoding="utf-8") as fh:
                    text = fh.read()
            except (UnicodeDecodeError, OSError):
                continue
            patched = patch(text)
            if patched != text:
                with open(path, "w", encoding="utf-8") as fh:
                    fh.write(patched)
    label = "iPhoneSimulator" if kind == "ios" else "AppleTVSimulator"
    print(f"{label} SDK -> {dst}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
