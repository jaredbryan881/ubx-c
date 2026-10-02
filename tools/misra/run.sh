#!/usr/bin/env bash
# Run cppcheck with the MISRA C:2012 addon over the library sources.
# Usage: tools/misra/run.sh [extra cppcheck args]
# The addon ships with cppcheck. Rule texts are copyrighted and not
# included; pass --rule-texts=<file> if you hold a licensed copy.
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$root"

# Some cppcheck builds ship without the MISRA addon, or the addon can be
# present but unusable in CI. Skip the check gracefully in that case instead of
# failing the whole job without any actionable diagnostics.
if ! cppcheck --addon=misra --help >/dev/null 2>&1; then
  echo "Skipping MISRA C:2012 check: cppcheck MISRA addon is unavailable in this environment." >&2
  exit 0
fi

exec cppcheck \
  --std=c99 \
  --enable=warning,style,performance,portability \
  --inconclusive \
  --addon=misra \
  --suppressions-list=tools/misra/suppressions.txt \
  --inline-suppr \
  --error-exitcode=1 \
  --template='{file}:{line}: [{id}] {message}' \
  -Iinclude -Isrc \
  "$@" \
  src
