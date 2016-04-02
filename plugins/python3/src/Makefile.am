AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(GNZBPY3_CFLAGS)

AM_CFLAGS = -Wall -I../ -I@top_srcdir@/../pywrap -I@top_srcdir@/../../src $(PTHREAD_CFLAGS)
AM_CXXFLAGS = -Wall -I../ -I@top_srcdir@/../pywrap -I@top_srcdir@/../../src $(PTHREAD_CFLAGS)

lib_LTLIBRARIES = libgnzbpy3.la

libgnzbpy3_la_SOURCES = \
	main.cpp \
	pygnzb.cpp \
	util.cpp \
	@top_srcdir@/../pywrap/pythonutil.cpp \
	@top_srcdir@/../pywrap/pythontypes.cpp \
	@top_srcdir@/../pywrap/pythonmodule.cpp \
	@top_srcdir@/../pywrap/pythonobject.cpp

libgnzbpy3_la_CFLAGS = $(GNZBPY3_CFLAGS) $(PYTHON_CFLAGS) $(AM_CFLAGS)
libgnzbpy3_la_CXXFLAGS = $(GNZBPY3_CXXFLAGS) $(PYTHON_CFLAGS) $(AM_CXXFLAGS)

libgnzbpy3_la_LIBADD = $(PYTHON_LIBS) 

## libgnzbpy3_la_LDFLAGS = -version-info @LIBVER_INFO@ $(PTHREAD_LIBS)
libgnzbpy3_la_LDFLAGS = -module -avoid-version $(PTHREAD_LIBS)
