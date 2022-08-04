#!/bin/bash
set -Eeuo pipefail

docker build -f "misc/Dockerfile" -t spectmorph-dbuild .
docker build -f "misc/Dockerfile-arch" -t spectmorph-dbuild-arch .
