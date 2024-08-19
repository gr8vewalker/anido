#!/bin/bash

premake5 clean
premake5 gmake2
CFLAGS='"-DANIDO_VERSION='"\\\"$(./scripts/version.sh)\\\""'"' make -C build config=${1:-debug}