SUBDIRS = src

gnzbpy3docdir = ${prefix}/doc/gnzb/plugins/python3

gnzbpy3doc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status libtool

# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(gnzbpy3docdir)


