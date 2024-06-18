#!/bin/bash
set -Eeuo pipefail

docker build -f "misc/Dockerfile" -t spectmorph-dbuild .
docker run -v $PWD:/output --rm -t spectmorph-dbuild misc/build.sh
