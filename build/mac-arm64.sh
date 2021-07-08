#!/bin/bash
set -euo pipefail
scripts/depsDownload.sh
ARCH="arm64v8" scripts/macSetup.sh "$@"
