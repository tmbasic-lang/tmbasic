#!/bin/bash
set -euxo pipefail
scripts/depsDownload.sh
ARCH="arm64v8" scripts/macSetup.sh "$@"
