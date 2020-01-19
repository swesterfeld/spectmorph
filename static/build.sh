#!/bin/bash
set -e
source ./config.sh
cd ..
docker build -t spectmorph -f static/Dockerfile .
docker run -v $PWD/static:/data -t spectmorph tar Cczfv /usr/local/spectmorph/lib/lv2 /data/spectmorph-$PACKAGE_VERSION-x86_64.tar.gz spectmorph.lv2
