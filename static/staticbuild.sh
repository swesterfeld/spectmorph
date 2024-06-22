#!/bin/bash
set -Eeuo pipefail

export CCACHE_DIR=/output/ccache
export CCACHE_MAXSIZE=1G

export CC="ccache gcc-13" CXX="ccache g++-13"

export PKG_CONFIG_PATH=/spectmorph/static/prefix/lib/pkgconfig:/spectmorph/static/prefix/lib/x86_64-linux-gnu/pkgconfig
export PKG_CONFIG="pkg-config --static"
cd static
./build-deps.sh
cd ..
export CPPFLAGS="-I/spectmorph/static/prefix/include"
./autogen.sh --prefix=/usr/local/spectmorph --with-static-cxx --without-qt --without-jack --without-ao --with-fonts --with-download-instruments
make clean
make -j$(nproc)
make -j$(nproc) check
sudo make install
cd tests
./post-install-test.sh
cd ..

cd lv2
rm -f spectmorph_lv2.so.static
make spectmorph_lv2.so.static
cd ..

cd vst
rm -f spectmorph_vst.so.static
make spectmorph_vst.so.static
cd ..

cd clap
rm -f SpectMorph.clap.static
make SpectMorph.clap.static
cd ..

# TEMPLATES / INSTRUMENTS
cp -av \
  /usr/local/spectmorph/share/spectmorph/templates \
  /usr/local/spectmorph/share/spectmorph/fonts \
  /data

if [ -d /usr/local/spectmorph/share/spectmorph/instruments ]; then
  cp -av /usr/local/spectmorph/share/spectmorph/instruments /data
fi

# VST
mkdir -p /data/vst
cp -av vst/spectmorph_vst.so.static /data/vst/spectmorph_vst.so

# CLAP
mkdir -p /data/clap
cp -av clap/SpectMorph.clap.static /data/clap/SpectMorph.clap

# LV2
mkdir -p /data/lv2/spectmorph.lv2
cp -av lv2/spectmorph_lv2.so.static /data/lv2/spectmorph.lv2/spectmorph_lv2.so
cp -av \
  /usr/local/spectmorph/lib/lv2/spectmorph.lv2/spectmorph.ttl \
  /usr/local/spectmorph/lib/lv2/spectmorph.lv2/manifest.ttl /data/lv2/spectmorph.lv2
