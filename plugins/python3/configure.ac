AC_INIT(gnzbpy3, 0.1)
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([subdir-objects])
AM_SILENT_RULES([yes])
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_CC
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX(14, [noext], [mandatory])
AX_PTHREAD()
AC_SEARCH_LIBS([dlopen], [dl dld], [], [AC_MSG_ERROR([unable to find the dlopen() function])])
AC_SEARCH_LIBS([dlsym], [dl dld], [], [AC_MSG_ERROR([unable to find the dlsym() function])])
AC_SEARCH_LIBS([dlclose], [dl dld], [], [AC_MSG_ERROR([unable to find the dlclose() function])])
PKG_CHECK_MODULES(GNZBPY3, [python3 libusenet])
PKG_CHECK_MODULES(PYVER, [python3])
LT_INIT([disable-static])
AC_SUBST([LIBTOOL_DEPS])
AC_SUBST(LIBVER_INFO, [1:0:0])

dnl define string for the python lib name
AC_DEFINE_UNQUOTED(PYLIB_NAME, [`echo $PYVER_LIBS | sed -e 's/-l//' -e 's/\(.*\)/"lib\1.so"/'`], [python library name])

AC_OUTPUT([
Makefile
src/Makefile
])

