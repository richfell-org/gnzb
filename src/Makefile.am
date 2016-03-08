AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(GNZB_CFLAGS)

AM_CFLAGS = -Wall
AM_CXXFLAGS = -Wall

bin_PROGRAMS = gnzb

gnzb_CFLAGS = -I../ -D_GNU_SOURCE -fno-strict-aliasing -fwrapv

gnzb_CXXFLAGS = -I../ -D_GNU_SOURCE -fno-strict-aliasing -fwrapv

gnzb_SOURCES = \
	main.cpp \
	application.cpp \
	application-gnzb.cpp \
	gnzb.cpp \
	runtimesettings.cpp \
	nntp/fetch.cpp \
	nntp/fetchmsg.cpp \
	nntp/articlefetchtask.cpp \
	nntp/articlefetchpool.cpp \
	gui/guignzb.cpp \
	gui/appwindow.cpp \
	gui/nzbtreeview.cpp \
	gui/nzbfiletreeview.cpp \
	gui/nzbliststore.cpp \
	gui/nzbfileliststore.cpp \
	gui/nzbdlinfoitem.cpp \
	gui/nzbdlinfobar.cpp \
	gui/nzbsummarywindow.cpp \
	gui/downloadspeedgraph.cpp \
	gui/statusbarwindow.cpp \
	gui/nzbfilechooser.cpp \
	gui/guiutil.cpp \
	gui/filedrophandler.cpp \
	gui/menus/appmenu.cpp \
	gui/menus/menuactions.cpp \
	gui/toolbar/toolbar.cpp \
	gui/toolbar/toolbutton.cpp \
	gui/toolbar/toggletoolbutton.cpp \
	gui/cellrenderer/gnzbcellrenderer.cpp \
	gui/cellrenderer/nzbfilecellrenderer.cpp \
	gui/cellrenderer/bulletpointrenderer.cpp \
	gui/cellrenderer/gnzbactioncellrenderer.cpp \
	gui/preferences/preferencespage.cpp \
	gui/preferences/preferencesinterface.cpp \
	gui/preferences/scriptsinterface.cpp \
	gui/preferences/locationsinterface.cpp \
	gui/preferences/nntpserverinterface.cpp \
	gui/preferences/notificationsinterface.cpp \
	gui/preferences/widgets/fsitemliststore.cpp \
	gui/preferences/widgets/fileliststore.cpp \
	gui/preferences/widgets/singlefileliststore.cpp \
	gui/preferences/widgets/folderselectionliststore.cpp \
	gui/preferences/widgets/nntpserverliststore.cpp \
	gui/groups/groupsliststore.cpp \
	gui/groups/groupsinterface.cpp \
	gui/groups/gnzbgroupsmenu.cpp \
	util/sizeformat.cpp \
	util/nzbutil.cpp \
	util/recttools.cpp \
	util/sysutil.cpp \
	db/nzbdb.cpp \
	db/sqlitedb.cpp \
	db/sqlite3.c \
	db/gnzbdb.cpp \
	db/preferences.cpp \
	db/gnzbgroups.cpp \
	uiresources.c

gnzb_LDFLAGS = 

gnzb_LDADD = $(GNZB_LIBS)

## glib resource compilier
## builds a resource section into the application binary
GLIB_COMPILE_RESOURCES=glib-compile-resources

## build resources into a source file which is included in the build
## using the --sourcedir option make this compatible with Autotools virtual builds
uiresources.c: uiresources.xml $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/uiresources.xml)
	$(GLIB_COMPILE_RESOURCES) --target=$@  --sourcedir=$(srcdir) --generate-source $<
