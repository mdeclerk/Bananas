#!/usr/bin/env bash
set -euo pipefail

IMAGE_NAME="bananas-buildenv"
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "==> Running make inside container"
docker run --rm \
  -v "${ROOT_DIR}:/bananas" \
  -w /bananas \
  "${IMAGE_NAME}" \
  make "$@"
