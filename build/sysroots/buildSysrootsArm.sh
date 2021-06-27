#!/bin/bash
DOCKER_ARCH="arm/v7" ./buildSysroot.sh
DOCKER_ARCH="arm/v8" ./buildSysroot.sh
