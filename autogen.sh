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

autoreconf -i && ./configure "$@"
