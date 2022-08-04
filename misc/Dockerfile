FROM gcc:latest

# kxstudio repos for lv2lint
RUN wget https://launchpad.net/~kxstudio-debian/+archive/kxstudio/+files/kxstudio-repos_11.1.0_all.deb
RUN dpkg -i kxstudio-repos_11.1.0_all.deb
RUN apt-get update && apt-get install -y \
  libsndfile1-dev libfftw3-dev libao-dev autoconf-archive clang \
  qtbase5-dev qtbase5-dev-tools qttools5-dev-tools \
  lv2-dev libjack-jackd2-dev lv2lint

ADD . /spectmorph
WORKDIR /spectmorph

RUN misc/build.sh
