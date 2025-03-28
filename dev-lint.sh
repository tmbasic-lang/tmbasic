#!/bin/bash
set -e
cpplint --quiet --recursive --repository=src src
