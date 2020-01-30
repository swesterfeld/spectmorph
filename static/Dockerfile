# build statically linked LV2/VST

FROM ubuntu:16.04

RUN sed -Ei 's/^# deb-src /deb-src /' /etc/apt/sources.list

RUN true && apt-get update && apt-get install -y build-essential
RUN apt-get install -y dpkg-dev
RUN apt-get install -y autoconf-archive
RUN apt-get install -y libtool
RUN apt-get install -y libjack-jackd2-dev
RUN apt-get install -y lv2-dev
RUN apt-get install -y curl
RUN apt-get install -y software-properties-common
RUN add-apt-repository ppa:ubuntu-toolchain-r/test
RUN apt-get update
RUN apt-get install -y g++-9
RUN apt-get install -y libtool-bin
RUN apt-get install -y gettext
RUN apt-get install -y quilt
RUN apt-get install -y libx11-dev
RUN apt-get install -y libxext-dev
RUN apt-get install -y mesa-common-dev
RUN apt-get install -y libgl-dev

ENV CC gcc-9
ENV CXX g++-9

ADD . /spectmorph
WORKDIR /spectmorph

ENV PKG_CONFIG_PATH=/spectmorph/static/prefix/lib/pkgconfig
ENV PKG_CONFIG="pkg-config --static"
RUN cd static && ./build-deps.sh
ENV CPPFLAGS="-I/spectmorph/static/prefix/include"
RUN ./autogen.sh --prefix=/usr/local/spectmorph --with-static-cxx --without-qt --without-jack --with-fonts
RUN make clean
RUN make -j16
RUN make install
RUN cd lv2 && rm -f spectmorph_lv2.so.static && make spectmorph_lv2.so.static
RUN cd vst && rm -f spectmorph_vst.so.static && make spectmorph_vst.so.static
