#!/bin/bash
BUILDX_ARCH="linux/386" IMAGE_ARCH="i386" ./buildSysroot.sh
BUILDX_ARCH="linux/amd64" IMAGE_ARCH="amd64" ./buildSysroot.sh
