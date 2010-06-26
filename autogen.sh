#!/bin/bash

if automake --version >/dev/null 2>&1; then
  :
else
  echo "You need to have automake installed to build this package"
  DIE=1
fi

if autoconf --version >/dev/null 2>&1; then
  :
else
  echo "You need to have autoconf installed to build this package"
  DIE=1
fi

if libtoolize --version >/dev/null 2>&1; then
  :
else
  echo "You need to have libtool installed to build this package"
  DIE=1
fi

# bail out as scheduled
test "0$DIE" -gt 0 && exit 1

echo "Ensure: ChangeLog"
test -e ChangeLog || TZ=GMT0 touch ChangeLog -t 190112132145.52 # automake *requires* ChangeLog

echo "Running: autoreconf -i && ./configure $@"
autoreconf -i -Wno-portability && ./configure "$@"
