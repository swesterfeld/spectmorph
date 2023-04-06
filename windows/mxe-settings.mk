## https://github.com/mxe/mxe.git  ec0b69a6527e3babf30a13d71a150449418c273b
MXE_TARGETS=x86_64-w64-mingw32.static
MXE_PLUGIN_DIRS=plugins/gcc12
JOBS=16

LOCAL_PKG_LIST := libsndfile glib libao cairo fftw
.DEFAULT_GOAL  := local-pkg-list
local-pkg-list: $(LOCAL_PKG_LIST)
