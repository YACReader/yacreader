#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

TEST_MODE=0
if [[ $# -gt 1 ]]; then
  echo "Unknown arguments. Supported flags: --test, -t" >&2
  exit 2
fi

if [[ $# -eq 1 ]]; then
  if [[ "${1:-}" == "--test" || "${1:-}" == "-t" ]]; then
    TEST_MODE=1
  else
    echo "Unknown argument: ${1:-}. Supported flags: --test, -t" >&2
    exit 2
  fi
fi

cd "${REPO_ROOT}"

if ! VERSION_OUTPUT="$(clang-format --version 2>/dev/null)"; then
  echo "Unable to run clang-format --version. Make sure clang-format is in PATH." >&2
  exit 127
fi

if [[ "${VERSION_OUTPUT}" =~ clang-format[[:space:]]+version[[:space:]]+([0-9]+(\.[0-9]+)*) ]]; then
  VERSION="${BASH_REMATCH[1]}"
  echo "Running using clang-format ${VERSION}"
else
  echo "Running using ${VERSION_OUTPUT}"
fi

find . \( -name '*.h' -or -name '*.cpp' -or -name '*.c' -or -name '*.mm' -or -name '*.m' \) -print0 | xargs -0 clang-format -style=file -i

if [[ "${TEST_MODE}" -eq 1 ]]; then
  BASE_SHA="$(git rev-parse HEAD)"
  git diff "${BASE_SHA}"
  if [[ "$(git diff "${BASE_SHA}")" != "" ]]; then
    exit 1
  fi
fi
