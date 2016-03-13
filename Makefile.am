# Check for any enable plugins
if PLUGIN_PY2
  PY2_DIR = plugins/python2
endif
if PLUGIN_PY3
  PY3_DIR = plugins/python3
endif
SUBDIRS = src $(PY2_DIR) $(PY3_DIR)

gnzbdocdirdir = ${prefix}/doc/gnzb

gnzbdocdir_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS

# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(gnzbdocdir)

