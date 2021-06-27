#!/bin/bash
DOCKER_ARCH=386 ./buildSysroot.sh
DOCKER_ARCH=amd64 ./buildSysroot.sh
