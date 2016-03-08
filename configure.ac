AC_INIT(gnzb, 0.1)
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([subdir-objects])
AM_SILENT_RULES([yes])
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_CC
AC_PROG_CXX
dnl AX_CXX_COMPILE_STDCXX_11
AX_CXX_COMPILE_STDCXX(14, [noext], [mandatory])
dnl AC_SEARCH_LIBS([dlopen], [dl dld], [], [AC_MSG_ERROR([unable to find the dlopen() function])])
AC_SEARCH_LIBS([dlsym], [dl dld], [], [AC_MSG_ERROR([unable to find the dlsym() function])])
AC_SEARCH_LIBS([dlclose], [dl dld], [], [AC_MSG_ERROR([unable to find the dlclose() function])])
PKG_CHECK_MODULES(GNZB, [gtkmm-3.0 gstreamermm-0.10 libnotify libusenet])

AC_OUTPUT([
Makefile
src/Makefile
])
