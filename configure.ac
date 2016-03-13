AC_INIT(gnzb, 0.1)
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([subdir-objects])
AM_SILENT_RULES([yes])
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_CC
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX(14, [noext], [mandatory])
dnl AC_SEARCH_LIBS([dlopen], [dl dld], [], [AC_MSG_ERROR([unable to find the dlopen() function])])
AC_SEARCH_LIBS([dlsym], [dl dld], [], [AC_MSG_ERROR([unable to find the dlsym() function])])
AC_SEARCH_LIBS([dlclose], [dl dld], [], [AC_MSG_ERROR([unable to find the dlclose() function])])
PKG_CHECK_MODULES(GNZB, [gtkmm-3.0 gstreamermm-0.10 libnotify libusenet])

dnl optional plugin for python2
AC_ARG_ENABLE([plugin-py2],
    AS_HELP_STRING([--enable-plugin-py2], [enable python2 scripting plugin]))
AS_IF(
	[test "x$enable_plugin_py2" == "xyes"],
	[
		AC_CONFIG_SUBDIRS([plugins/python2])
		AC_DEFINE(GNZB_PLUGIN_PY2, [], [Include plugin for Python2 script support])
	],
	[])

dnl optional plugin for python3
AC_ARG_ENABLE([plugin-py3],
    AS_HELP_STRING([--enable-plugin-py3], [enable python3 scripting plugin]))
AS_IF(
	[test "x$enable_plugin_py3" == "xyes"],
	[
		AC_CONFIG_SUBDIRS([plugins/python3])
		AC_DEFINE(GNZB_PLUGIN_PY3, [], [Include plugin for Python3 script support])
	],
	[])

dnl defines for the enabled plugins used by Makefile.am
AM_CONDITIONAL([PLUGIN_PY2], [test "x$enable_plugin_py2" == "xyes"])
AM_CONDITIONAL([PLUGIN_PY3], [test "x$enable_plugin_py3" == "xyes"])

dnl we need to export the app's symbols if a plugin is enabled
dnl because the plugins use symbols defined in the app
AS_IF(
	[test "x$enable_plugin_py3" == "xyes" -o "x$enable_plugin_py2" == "xyes"],
	[
		GNZB_LDFLAGS="$GNZB_LDFLAGS -Wl,--export-dynamic"
		AC_SUBST(GNZB_LDFLAGS)
		AC_DEFINE(PLUGINS_ENABLED, [], [At least one plugin module is enabled])
	],
	[])

AC_OUTPUT([
Makefile
src/Makefile
])