#!/bin/bash

branch=$(git branch --show-current)
commit=$(git describe --always)
echo "$branch/$commit"
