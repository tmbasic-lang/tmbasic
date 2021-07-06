#!/bin/bash
set -euo pipefail
files/depsDownload.sh
ARCH="arm64v8" scripts/macSetup.sh "$@"
