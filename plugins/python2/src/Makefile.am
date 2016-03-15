AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(GNZBPY2_CFLAGS)

AM_CFLAGS = -Wall -I../ -I@top_srcdir@/../../src $(PTHREAD_CFLAGS)
AM_CXXFLAGS = -Wall -I../ -I@top_srcdir@/../../src $(PTHREAD_CFLAGS)

lib_LTLIBRARIES = libgnzbpy2.la

libgnzbpy2_la_SOURCES = \
	main.cpp \
	util.cpp \
	module/gnzbmodule.cpp

libgnzbpy2_la_CFLAGS = $(GNZBPY2_CFLAGS) $(PYTHON_CFLAGS) $(AM_CFLAGS)
libgnzbpy2_la_CXXFLAGS = $(GNZBPY2_CXXFLAGS) $(PYTHON_CFLAGS) $(AM_CXXFLAGS)

## libgnzbpy2_la_LIBADD = 

## libgnzbpy2_la_LDFLAGS = -version-info @LIBVER_INFO@ $(PTHREAD_LIBS)
libgnzbpy2_la_LDFLAGS = -module -avoid-version $(PTHREAD_LIBS)
