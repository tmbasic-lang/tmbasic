#!/bin/bash
set -euxo pipefail

MACTRIPLE="arm64-apple-macos11" \
    MACVER="11.0" \
    MACARCH="arm64" \
    SHORT_ARCH="arm64" \
    DOCKER_ARCH="arm64v8" \
    scripts/macSetup.sh
