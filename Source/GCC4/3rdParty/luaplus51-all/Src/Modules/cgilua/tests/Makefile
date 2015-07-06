# $Id: Makefile,v 1.5 2004/11/08 16:39:51 tomas Exp $

T= test

include ../config

V= 1.0a
#DIST_DIR=../$(PKG)/$T

SRCS= test_main.html test_main.lua test_main.lp test_cookies.lua test_cookies.lp

clean cgi fcgi mod:

cgiinstall fcgiinstall modinstall:

dist dist_dir:
	mkdir -p $(DIST_DIR)
	cp $(SRCS) Makefile $(DIST_DIR)
