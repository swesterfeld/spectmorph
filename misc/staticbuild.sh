#!/bin/bash

set -e

./autogen.sh
cd static
source ./config.sh
cd ..
docker build -t spectmorph -f static/Dockerfile .
