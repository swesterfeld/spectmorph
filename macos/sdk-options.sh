SDK_DIRECTORY="$HOME/MacOSX11.3.sdk"
SDK_MINVERSION="11"
if [ x$SDK_TARGET = "xx86_64" ]; then
  SDK_OPTIONS="-isysroot $SDK_DIRECTORY -mmacosx-version-min=$SDK_MINVERSION -target x86_64-apple-macos11"
  SDK_AUTOCONF_BUILD="--build=x86_64-apple-darwin"
elif [ x$SDK_TARGET = "xaarch64" ]; then
  SDK_OPTIONS="-isysroot $SDK_DIRECTORY -mmacosx-version-min=$SDK_MINVERSION -target aarch64-apple-macos11"
  SDK_AUTOCONF_BUILD="--build=aarch64-apple-darwin"
else
  :
fi

# Compatibility: https://en.wikipedia.org/wiki/Xcode#Version_comparison_table
# Download SDKs: https://github.com/phracker/MacOSX-SDKs
