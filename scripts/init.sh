#!/usr/bin/env bash
set -euo pipefail

IMAGE_NAME="bananas-buildenv"
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

if docker image inspect "${IMAGE_NAME}" &>/dev/null; then
    echo "==> Image '${IMAGE_NAME}' already exists, skipping build. Use --force to rebuild."
    if [[ "${1:-}" != "--force" ]]; then
        exit 0
    fi
    echo "==> --force specified, rebuilding..."
fi

echo "==> Building Docker image: ${IMAGE_NAME}"
docker build -t "${IMAGE_NAME}" "${ROOT_DIR}"
echo "==> Done."
