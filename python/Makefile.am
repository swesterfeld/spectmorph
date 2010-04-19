EXTRA_DIST = sinsignal.py

AM_CXXFLAGS = $(PYTHON_INCLUDES) -I$(top_srcdir)/lib $(BSE_CFLAGS)

pyexec_LTLIBRARIES = spectmorph.la
spectmorph_la_SOURCES = spectmorph.cc
spectmorph_la_LDFLAGS = -avoid-version -module
spectmorph_la_LIBADD = $(top_builddir)/lib/libspectmorph.la

#all: spectmorph.so
#
#spectmorph.so: spectmorph.cc
#	python setup.py build_ext --inplace -I$(top_srcdir)/lib$(shell for i in $(BSE_CFLAGS); do echo -n $$(echo $$i | grep '^-I' | sed s/-I/:/g); done) -lspectmorph
