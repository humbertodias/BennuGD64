#!/bin/sh
# Browser/GitHub downloads set com.apple.quarantine; dyld then refuses @rpath dylibs.
DIR=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
xattr -cr "$DIR" 2>/dev/null || true
exec "$DIR/bgdi" "$@"
