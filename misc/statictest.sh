#!/usr/bin/bash
set -e
source static/config.sh

SPECTMORPH_DIR="$HOME/.local/share/spectmorph"

# missing: instruments
mkdir -p "$SPECTMORPH_DIR"
cp -av /data/templates /data/fonts "$SPECTMORPH_DIR"

# kxstudio repos for lv2lint
sudo apt-get install -y wget
wget https://launchpad.net/~kxstudio-debian/+archive/kxstudio/+files/kxstudio-repos_11.1.0_all.deb
sudo dpkg -i kxstudio-repos_11.1.0_all.deb
sudo apt-get update
sudo apt-get install -y lv2lint

# test static plugin
export LV2_PATH=/data/lv2:/usr/lib/lv2
lv2lint http://spectmorph.org/plugins/spectmorph
