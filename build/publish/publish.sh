#!/bin/bash
set -euxo pipefail

ssh -i $ARM_KEY $ARM_USER@$ARM_HOST "bash -s" < 1-arm-runners.sh
ssh -i $X64_KEY $X64_USER@$X64_HOST "bash -s" < 2-intel-runners.sh
