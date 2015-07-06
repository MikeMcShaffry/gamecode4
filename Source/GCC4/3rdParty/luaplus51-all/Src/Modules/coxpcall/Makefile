# $Id: Makefile,v 1.3 2008/01/22 18:39:18 mascarenhas Exp $

config_file:=config

ifneq '$(wildcard $(config_file))' ''
include $(config_file)
endif

$(config_file):
	chmod +x configure

install: $(config_file)
	mkdir -p $(LUA_DIR)
	cp src/coxpcall.lua $(LUA_DIR)/

install-rocks: install
	mkdir -p $(PREFIX)/doc
	cp -r doc/* $(PREFIX)/doc
	echo "Go to $(PREFIX) for docs!"

