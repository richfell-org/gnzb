SUBDIRS = src

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

