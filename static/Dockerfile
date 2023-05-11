# build statically linked LV2/VST/CLAP plugin

FROM ubuntu:20.04

RUN sed -Ei 's/^# deb-src /deb-src /' /etc/apt/sources.list

RUN apt-get update
RUN apt-get install -y software-properties-common
RUN add-apt-repository ppa:ubuntu-toolchain-r/test
RUN apt-get update && apt-get install -y build-essential \
 dpkg-dev autoconf-archive libtool libjack-jackd2-dev lv2-dev \
 curl g++-11 libtool-bin gettext quilt libx11-dev libxext-dev \
 mesa-common-dev libgl-dev meson python3-setuptools

ENV CC gcc-11
ENV CXX g++-11

ADD . /spectmorph
WORKDIR /spectmorph

ENV PKG_CONFIG_PATH=/spectmorph/static/prefix/lib/pkgconfig:/spectmorph/static/prefix/lib/x86_64-linux-gnu/pkgconfig
ENV PKG_CONFIG="pkg-config --static"
RUN cd static && ./build-deps.sh
ENV CPPFLAGS="-I/spectmorph/static/prefix/include"
RUN ./autogen.sh --prefix=/usr/local/spectmorph --with-static-cxx --without-qt --without-jack --without-ao --with-fonts
RUN make clean
RUN make -j16
RUN make install
RUN cd lv2 && rm -f spectmorph_lv2.so.static && make spectmorph_lv2.so.static
RUN cd vst && rm -f spectmorph_vst.so.static && make spectmorph_vst.so.static
RUN cd clap && rm -f SpectMorph.clap.static && make SpectMorph.clap.static
