if [ x$SDK_TARGET = "xx86_64" ]; then
  SDK_DIRECTORY="$HOME/MacOSX10.9.sdk"
  SDK_MINVERSION="10.9"
  SDK_OPTIONS="-isysroot $SDK_DIRECTORY -mmacosx-version-min=$SDK_MINVERSION -target x86_64-apple-macos10.9"
  SDK_AUTOCONF_BUILD="--build=x86_64-apple-darwin"
else
  :
fi
