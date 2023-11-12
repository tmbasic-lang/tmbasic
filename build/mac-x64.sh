#!/bin/bash
set -euxo pipefail

# Verify that the host macOS system is ARM64, NOT Intel.
echo "Host type: $(uname -m)"
if [ "$(uname -m)" != "arm64" ]; then
    echo "This script must be run on an Apple Silicon Mac."
    exit 1
fi

ARCH="x86_64" scripts/macSetup.sh "$@"
