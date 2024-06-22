# build statically linked LV2/VST/CLAP plugin

FROM ubuntu:20.04

RUN sed -Ei 's/^# deb-src /deb-src /' /etc/apt/sources.list

RUN apt-get update
RUN apt-get install -y software-properties-common
RUN add-apt-repository ppa:ubuntu-toolchain-r/test
RUN apt-get update && apt-get install -y build-essential \
 dpkg-dev autoconf-archive libtool libjack-jackd2-dev lv2-dev \
 curl g++-13 libtool-bin gettext quilt libx11-dev libxext-dev \
 mesa-common-dev libgl-dev meson python3-setuptools ccache sudo \
 python3-scipy

ADD . /spectmorph
WORKDIR /spectmorph

# Become non-root
#RUN groupadd --gid 1000 builder \
#  && useradd --uid 1000 --gid 1000 --create-home --shell /bin/bash builder \
#  && echo "builder ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/builder \
#  && chown -R builder:builder /spectmorph
#USER builder
