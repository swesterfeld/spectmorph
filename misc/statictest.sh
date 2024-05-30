#!/usr/bin/bash
set -e
# test static plugin
mkdir -p /usr/lib/lv2/spectmorph.lv2
cp -av lv2/spectmorph_lv2.so.static /usr/lib/lv2/spectmorph.lv2/spectmorph_lv2.so
cp -av lv2/spectmorph.ttl lv2/manifest.ttl /usr/lib/lv2/spectmorph.lv2
# kxstudio repos for lv2lint
apt-get install -y wget
wget https://launchpad.net/~kxstudio-debian/+archive/kxstudio/+files/kxstudio-repos_11.1.0_all.deb
dpkg -i kxstudio-repos_11.1.0_all.deb
apt-get update
apt-get install -y lv2lint
lv2lint http://spectmorph.org/plugins/spectmorph
