#!/bin/bash

set -e

sudo apt-get install -y gettext
./autogen.sh
cd static
source ./config.sh
cd ..
docker build -t spectmorph -f static/Dockerfile .
