#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ROM="${ROOT_DIR}/build/bananas.gb"

if [[ ! -f "${ROM}" ]]; then
  echo "Error: ROM not found at ${ROM}" >&2
  echo "Run ./scripts/build.sh first." >&2
  exit 1
fi

case "$(uname -s)" in
  Darwin)
    if ! open -a SameBoy "${ROM}" 2>/dev/null; then
      echo "Error: SameBoy not found." >&2
      echo "Install SameBoy: brew install --cask sameboy" >&2
      exit 1
    fi
    ;;

  Linux)
    if command -v sameboy &>/dev/null; then
      sameboy "${ROM}"
    else
      echo "Error: SameBoy not found." >&2
      echo "Install SameBoy: https://sameboy.github.io" >&2
      exit 1
    fi
    ;;

  MINGW*|MSYS*|CYGWIN*)
    WIN_ROM="$(cygpath -w "${ROM}" 2>/dev/null || echo "${ROM}")"
    if command -v sameboy.exe &>/dev/null; then
      sameboy.exe "${WIN_ROM}"
    elif command -v SameBoy.exe &>/dev/null; then
      SameBoy.exe "${WIN_ROM}"
    else
      echo "Error: SameBoy not found in PATH." >&2
      echo "Install SameBoy: https://sameboy.github.io" >&2
      echo "Then add the SameBoy directory to your PATH." >&2
      exit 1
    fi
    ;;

  *)
    echo "Error: Unsupported OS: $(uname -s)" >&2
    exit 1
    ;;
esac
