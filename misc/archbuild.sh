#!/bin/bash
set -Eeuo pipefail

docker build -f "misc/Dockerfile-arch" -t spectmorph-dbuild-arch .
