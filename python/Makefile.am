include $(top_srcdir)/Makefile.decl

EXTRA_DIST += sinsignal.py

AM_CXXFLAGS = $(PYTHON_INCLUDES) -I$(top_srcdir)/lib $(BSE_CFLAGS) $(SSE_FLAGS)

pyexec_LTLIBRARIES = spectmorph.la
spectmorph_la_SOURCES = spectmorph.cc
spectmorph_la_LDFLAGS = -avoid-version -module
spectmorph_la_LIBADD = $(top_builddir)/lib/libspectmorph.la
