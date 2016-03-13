AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(GNZBPY2_CFLAGS)

AM_CFLAGS = -Wall -I@top_srcdir@/../pycxx-6.2.6 -I@top_srcdir@/../../src $(PTHREAD_CFLAGS)
AM_CXXFLAGS = -Wall -I@top_srcdir@/../pycxx-6.2.6 -I@top_srcdir@/../../src $(PTHREAD_CFLAGS)

lib_LTLIBRARIES = libgnzbpy2.la

libgnzbpy2_la_SOURCES = \
	main.cpp \
	module/gnzbmodule.cpp \
	pyobject/gnzbpyobject.cpp \
	IndirectPythonInterface.cxx \
	@top_srcdir@/../pycxx-6.2.6/Src/Python2/cxxextensions.c \
	@top_srcdir@/../pycxx-6.2.6/Src/Python2/cxx_extensions.cxx \
	@top_srcdir@/../pycxx-6.2.6/Src/Python2/cxxsupport.cxx

libgnzbpy2_la_CFLAGS = $(GNZBPY2_CFLAGS) $(AM_CFLAGS)
libgnzbpy2_la_CXXFLAGS = $(GNZBPY2_CXXFLAGS) $(AM_CXXFLAGS)

libgnzbpy2_la_LIBADD = $(GNZBPY2_LIBS)

## libgnzbpy2_la_LDFLAGS = -version-info @LIBVER_INFO@ $(PTHREAD_LIBS)
libgnzbpy2_la_LDFLAGS = -module -avoid-version $(PTHREAD_LIBS)
