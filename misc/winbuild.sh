#!/bin/bash
set -Eeuo pipefail

docker build -f "misc/Dockerfile-win" -t spectmorph-dbuild-win .
docker run -v $PWD:/data -t spectmorph-dbuild-win cp -av /usr/local/spectmorph-win/lib/vst/SpectMorph.dll /data/SpectMorph.dll
