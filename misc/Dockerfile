FROM gcc:latest

RUN apt-get update && apt-get install -y \
  libsndfile1-dev libfftw3-dev libao-dev autoconf-archive clang ccache \
  qtbase5-dev qtbase5-dev-tools qttools5-dev-tools \
  lv2-dev libjack-jackd2-dev gettext python3-scipy meson \
  liblilv-dev libelf-dev

ADD . /spectmorph
WORKDIR /spectmorph
