#!/bin/bash
set -euxo pipefail

MACTRIPLE="x86_64-apple-macos10.13" \
    MACVER="10.13" \
    MACARCH="x86_64" \
    SHORT_ARCH="x64" \
    DOCKER_ARCH="x86_64" \
    scripts/macSetup.sh
