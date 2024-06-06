#!/bin/bash

set -e

sudo apt-get install -y gettext libsndfile1-dev libfftw3-dev libgl-dev
./autogen.sh --without-ao
cd static
source ./config.sh
cd ..
docker build -t spectmorph-static -f static/Dockerfile .
docker run -t spectmorph-static misc/statictest.sh