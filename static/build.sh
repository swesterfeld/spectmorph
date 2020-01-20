#!/bin/bash
set -e
source ./config.sh
cd ..
docker build -t spectmorph -f static/Dockerfile .
mkdir -p static/bin
docker run -v $PWD/static:/data -t spectmorph cp -av lv2/*.ttl vst/spectmorph_vst.so.static lv2/spectmorph_lv2.so.static /data/bin
