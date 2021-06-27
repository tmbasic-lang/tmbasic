#!/bin/bash
BUILDX_ARCH="linux/arm/v7" IMAGE_ARCH="arm32v7" ./buildSysroot.sh
BUILDX_ARCH="linux/arm/v8" IMAGE_ARCH="arm64v8" ./buildSysroot.sh
