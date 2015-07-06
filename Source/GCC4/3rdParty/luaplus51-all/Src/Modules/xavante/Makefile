# $Id: Makefile,v 1.45 2008/04/04 20:00:00 mascarenhas Exp $

CONFIG= ./config

include $(CONFIG)

XAVANTE_START= src/xavante_start
SAJAX_LUAS = src/sajax/sajax.lua
ROOT_LUAS = src/xavante/xavante.lua 
XAVANTE_LUAS= src/xavante/cgiluahandler.lua src/xavante/encoding.lua src/xavante/filehandler.lua src/xavante/httpd.lua src/xavante/mime.lua src/xavante/patternhandler.lua src/xavante/redirecthandler.lua src/xavante/vhostshandler.lua src/xavante/indexhandler.lua src/xavante/urlhandler.lua src/xavante/ruleshandler.lua
DOCS= doc/us/index.html doc/us/license.html doc/us/manual.html doc/us/sajax.html doc/us/xavante.gif

all:

install:
	mkdir -p $(LUA_DIR)/xavante
	cp $(ROOT_LUAS) $(SAJAX_LUAS) $(LUA_DIR)
	cp $(XAVANTE_LUAS) $(LUA_DIR)/xavante

clean:
