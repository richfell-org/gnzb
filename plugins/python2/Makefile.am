SUBDIRS = src

gnzbpy2docdir = ${prefix}/doc/gnzb/plugins/python2

gnzbpy2doc_DATA = \
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
	-rm -r $(gnzbpy2docdir)