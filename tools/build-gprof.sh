SOURCES="lib/smafile.cc lib/smencoder.cc lib/smframe.cc lib/smnoisedecoder.cc lib/smsinedecoder.cc"
CXXFLAGS="-O2 -g -pg"
cd ..
set -x
for tool in smplay smenc
do
  gcc -c $SOURCES src/${tool}.cc $(pkg-config --cflags gtk+-2.0 bse) -I /usr/local/boost-numeric-bindings/include/boost-numeric-bindings/ -I lib -I src -I . || exit "$@"
  gcc -o $tool smafile.o smencoder.o smframe.o smnoisedecoder.o smsinedecoder.o ${tool}.o -g -pg -lao -llapack $(pkg-config --libs gtk+-2.0 bse) || exit "$@"
done
