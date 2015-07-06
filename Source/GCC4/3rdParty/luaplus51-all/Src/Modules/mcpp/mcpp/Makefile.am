## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign

SUBDIRS = src

if REPLACE_CPP
SUBDIRS += tests .
man_page = mcpp-gcc
else
man_page = mcpp
endif
man1_MANS = $(man_page).1

EXTRA_DIST = $(top_srcdir)/LICENSE $(top_srcdir)/INSTALL*   \
  $(top_srcdir)/ChangeLog.old $(top_srcdir)/NEWS    \
  $(top_srcdir)/mcpp.1 $(top_srcdir)/mcpp-gcc.1 $(top_srcdir)/config    \
  $(top_srcdir)/src/preproc.c $(top_srcdir)/src/noconfig.H  \
  $(top_srcdir)/src/cc1.c $(top_srcdir)/src/*set_mcpp.sh 	\
  $(top_srcdir)/src/testmain.c $(top_srcdir)/src/mcpp_lib.def	\
  $(top_srcdir)/tests/*_test.sh $(top_srcdir)/noconfig  \
  $(top_srcdir)/cpp-test $(top_srcdir)/test-t    \
  $(top_srcdir)/test-c $(top_srcdir)/test-l $(top_srcdir)/tool  \
  $(top_srcdir)/doc $(top_srcdir)/doc-jp

if ! REPLACE_CPP 
install-exec-hook:
	$(top_srcdir)/config/install-sh -d "$(DESTDIR)$(prefix)/share/doc/mcpp"
	cp -pf $(top_srcdir)/LICENSE $(top_srcdir)/README $(top_srcdir)/NEWS \
            $(DESTDIR)$(prefix)/share/doc/mcpp
	cp -pf $(top_srcdir)/doc/mcpp-manual.html   \
            $(DESTDIR)$(prefix)/share/doc/mcpp
	cp -pf $(top_srcdir)/doc-jp/mcpp-manual.html   \
            $(DESTDIR)$(prefix)/share/doc/mcpp/mcpp-manual-jp.html
uninstall-hook:
	rm -rf $(DESTDIR)$(prefix)/share/doc/mcpp
endif

