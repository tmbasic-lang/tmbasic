#!/bin/bash
set -euo pipefail
files/depsDownload.sh
ARCH="x86_64" scripts/macSetup.sh "$@"
