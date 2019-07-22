noinst_LTLIBRARIES = libminizip.la

EXTRA_DIST = LICENSE

MINIZIP_SRC = mz_crypt.c mz_os.c mz_strm.c mz_strm_buf.c mz_strm_mem.c mz_strm_split.c mz_zip.c mz_zip_rw.c mz_strm_zlib.c
MINIZIP_HDRS = mz.h mz_os.h mz_crypt.h mz_strm.h mz_strm_buf.h mz_strm_mem.h mz_strm_split.h mz_strm_os.h mz_zip.h mz_zip_rw.h mz_strm_zlib.h

if COND_WINDOWS
  MINIZIP_SRC += mz_os_win32.c mz_strm_os_win32.c
else
  MINIZIP_SRC += mz_os_posix.c mz_strm_os_posix.c
endif

libminizip_la_SOURCES = $(MINIZIP_SRC) $(MINIZIP_HDRS)
libminizip_la_CFLAGS = $(AM_CFLAGS) -DHAVE_STDINT_H -DMZ_ZIP_NO_ENCRYPTION -DHAVE_ZLIB -D_POSIX_C_SOURCE=200112L
libminizip_la_LIBADD = $(ZLIB_LIBS)
