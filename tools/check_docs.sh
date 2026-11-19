#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

fail=0
err() { echo "FAIL: $*" >&2; fail=1; }

for f in tests/test_*.cpp; do
  [ -e "$f" ] || continue
  name=$(basename "$f")
  if ! grep -q "$name" CMakeLists.txt; then
    err "$f not listed in CMakeLists.txt"
  fi
done

md_files=$(find . -name '*.md' \
  -not -path './plan/*' \
  -not -path './build/*' \
  -not -path './english/*'\
  -not -path './knowledge/*'\
  -not -path './build-*/*' \
  -not -path './.git/*')

for f in $md_files; do
  if grep -q '<!-- historical -->' "$f"; then
    continue
  fi

  tokens=$(grep -v 'PLANNED' "$f" | grep -oE '(src|include|tests|tools|docs|notes)/[A-Za-z0-9_./-]+\.[A-Za-z0-9]+' | sort -u || true)
  for token in $tokens; do
    if [ ! -f "$token" ]; then
      err "$f: missing file referenced '$token'"
    fi
  done

  links=$(grep -v 'PLANNED' "$f" | grep -oE '\]\([^)]+\)' | sed 's/^](//; s/)$//' | sort -u || true)
  for link in $links; do
    case "$link" in
      http://*|https://*|mailto:*|\#*) continue ;;
    esac
    target="${link%%#*}"
    [ -z "$target" ] && continue

    dir=$(dirname "$f")
    if [ ! -e "$dir/$target" ] && [ ! -e "$target" ]; then
      err "$f: broken link ']($link)'"
    fi
  done
done

if [ "$fail" -ne 0 ]; then
  echo "check_docs: FAILED" >&2
  exit 1
fi
echo "check_docs: OK"