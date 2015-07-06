# $Id: Makefile,v 1.14 2008/04/04 02:01:01 mascarenhas Exp $

include config

all: cgi fastcgi

cgi:

config:
	touch config

fastcgi: src/fastcgi/lfcgi.so

fcgi: fastcgi

src/fastcgi/lfcgi.so: src/fastcgi/lfcgi.o src/fastcgi/lfcgi.h
	$(CC) $(CFLAGS) $(LIB_OPTION) -o src/fastcgi/lfcgi.so src/fastcgi/lfcgi.o -lfcgi 

install:
	mkdir -p $(LUA_DIR)/wsapi
	cp src/wsapi/*.lua $(LUA_DIR)/wsapi
	cp src/launcher/wsapi.cgi $(BIN_DIR)/
	cp src/launcher/wsapi.fcgi $(BIN_DIR)/

install-fcgi:
	cp src/fastcgi/lfcgi.so $(LUA_LIBDIR)/

install-rocks: install
	mkdir -p $(PREFIX)/samples
	cp -r samples/* $(PREFIX)/samples
	mkdir -p $(PREFIX)/doc
	cp -r doc/* $(PREFIX)/doc

clean:
	rm src/fastcgi/lfcgi.o src/fastcgi/lfcgi.so

snapshot:
	git archive --format=tar --prefix=wsapi-$(VERSION)/ HEAD | gzip > wsapi-$(VERSION).tar.gz

rockspecs:
	for pkg in wsapi wsapi-fcgi wsapi-xavante ; do cp rockspec/$$pkg-$(VERSION_OLD)-1.rockspec rockspec/$$pkg-$(VERSION_NEW)-1.rockspec ; done
	for pkg in wsapi wsapi-fcgi wsapi-xavante; do sed -e "s/$(VERSION_OLD)/$(VERSION_NEW)/g" -i "" rockspec/$$pkg-$(VERSION_NEW)-1.rockspec ; done
	for pkg in wsapi wsapi-fcgi wsapi-xavante; do git add rockspec/$$pkg-$(VERSION_NEW)-1.rockspec ; done
