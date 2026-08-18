#!/usr/bin/env bash
# Generate Doxygen HTML under doc/html/ (see doc/pages.yml).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DOC="$ROOT/doc"
cd "$DOC"

if ! command -v doxygen >/dev/null 2>&1; then
  echo "doxygen: not found (install doxygen and optionally graphviz)" >&2
  exit 1
fi

VERSION="dev"
if command -v git >/dev/null 2>&1 && git -C "$ROOT" rev-parse --git-dir >/dev/null 2>&1; then
  VERSION="$(git -C "$ROOT" describe --tags --always --dirty 2>/dev/null || echo dev)"
fi

cat > .doxygen-extra <<EOF
PROJECT_NUMBER = ${VERSION}
EOF

doxygen Doxyfile

test -f html/index.html
echo "Doxygen output: doc/html/ (${VERSION})"
