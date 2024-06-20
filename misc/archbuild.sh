#!/bin/bash
set -Eeuo pipefail

docker build -f "misc/Dockerfile-arch" -t spectmorph-dbuild-arch .
docker run -v $PWD:/output --rm -t spectmorph-dbuild-arch misc/build.sh "$@"
