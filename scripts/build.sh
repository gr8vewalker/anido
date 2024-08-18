#!/bin/bash

premake5 gmake2
make -C build config=${1:-debug}