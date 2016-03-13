AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(GNZBPY3_CFLAGS)

AM_CFLAGS = -Wall -I@top_srcdir@/../pycxx-6.2.6 -I@top_srcdir@/../../src $(PTHREAD_CFLAGS)
AM_CXXFLAGS = -Wall -I@top_srcdir@/../pycxx-6.2.6 -I@top_srcdir@/../../src $(PTHREAD_CFLAGS)

lib_LTLIBRARIES = libgnzbpy3.la

libgnzbpy3_la_SOURCES = \
	main.cpp \
	IndirectPythonInterface.cxx \
	module/gnzbmodule.cpp \
	pyobject/gnzbpyobject.cpp \
	@top_srcdir@/../pycxx-6.2.6/Src/Python3/cxxextensions.c \
	@top_srcdir@/../pycxx-6.2.6/Src/Python3/cxx_extensions.cxx \
	@top_srcdir@/../pycxx-6.2.6/Src/Python3/cxxsupport.cxx

libgnzbpy3_la_CFLAGS = $(GNZBPY3_CFLAGS) $(AM_CFLAGS)
libgnzbpy3_la_CXXFLAGS = $(GNZBPY3_CXXFLAGS) $(AM_CXXFLAGS)

libgnzbpy3_la_LIBADD = $(GNZBPY3_LIBS)

## libgnzbpy3_la_LDFLAGS = -version-info @LIBVER_INFO@ $(PTHREAD_LIBS)
libgnzbpy3_la_LDFLAGS = -module -avoid-version $(PTHREAD_LIBS)