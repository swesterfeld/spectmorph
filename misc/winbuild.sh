#!/bin/bash
set -Eeo pipefail

docker build -f "misc/Dockerfile-win" -t spectmorph-dbuild-win .
if [ "x$1" == "xsetup" ]; then
  docker run -v $PWD:/data -t spectmorph-dbuild-win bash -c '
    cd windows
    ./build.sh
    ./mk-setup.sh
    source ./config.sh
    cp -av setup-spectmorph-${PACKAGE_VERSION}.exe /data'
else
  docker run -v $PWD:/data -t spectmorph-dbuild-win cp -av /usr/local/spectmorph-win/lib/vst/SpectMorph.dll /data/
  docker run -v $PWD:/data -t spectmorph-dbuild-win cp -av /usr/local/spectmorph-win/lib/clap/SpectMorph.clap /data/
  docker run -v $PWD:/data -t spectmorph-dbuild-win cp -rav /usr/local/spectmorph-win/lib/lv2/spectmorph.lv2 /data/
fi
