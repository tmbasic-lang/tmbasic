#!/bin/bash
DOCKER_ARCH=arm32v7 ./buildSysroot.sh
DOCKER_ARCH=arm64v8 ./buildSysroot.sh
