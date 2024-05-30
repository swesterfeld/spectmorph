#!/bin/bash

set -e

sudo apt-get install -y gettext libsndfile1-dev libfftw3-dev
./autogen.sh --without-ao
cd static
source ./config.sh
cd ..
docker build -t spectmorph -f static/Dockerfile .
