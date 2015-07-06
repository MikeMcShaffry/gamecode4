# $Id: Makefile,v 1.1.1.1 2007/11/26 17:12:24 mascarenhas Exp $

config_file:=config

ifneq '$(wildcard $(config_file))' ''
include $(config_file)
endif

$(config_file):
	chmod +x configure

all:

install: $(config_file)
	mkdir -p $(LUA_DIR)
	mkdir -p $(LUA_DIR)/cosmo
	cp src/cosmo.lua $(LUA_DIR)/
	cp src/cosmo/grammar.lua $(LUA_DIR)/cosmo
	cp src/cosmo/fill.lua $(LUA_DIR)/cosmo

install-rocks: install
	mkdir -p $(PREFIX)/samples
	cp -r samples/* $(PREFIX)/samples
	mkdir -p $(PREFIX)/doc
	cp -r doc/* $(PREFIX)/doc
	mkdir -p $(PREFIX)/tests
	cp -r tests/* $(PREFIX)/tests
	echo "Go to $(PREFIX) for samples and docs!"

test:
	cd tests && lua -l luarocks.require test_cosmo.lua

upload-cvs:
	git archive --format=tar --prefix=cosmo-current/ master | gzip > cosmo-current.tar.gz
	ncftpput -u mascarenhas ftp.luaforge.net cosmo/htdocs cosmo-current.tar.gz

upload-dist:
	git archive --format=tar --prefix=cosmo-$(VERSION)/ v$(VERSION) | gzip > cosmo-$(VERSION).tar.gz
	ncftpput -u mascarenhas ftp.luaforge.net cosmo/htdocs cosmo-$(VERSION).tar.gz
	ncftpput -u mascarenhas ftp.luaforge.net cosmo/htdocs doc/index.html

clean:
