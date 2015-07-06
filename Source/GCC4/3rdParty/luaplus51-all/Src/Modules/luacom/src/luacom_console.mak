#
# tecmake for LuaCOM test console
#


include luacom.inc

ifeq "$(TEC_UNAME)" "dll"
APPNAME = luacom$(LUA_VERSION_NUMBER)-dll-test
USE_DLL=YES
else
APPNAME = luacom$(LUA_VERSION_NUMBER)-lib-test
endif

ifdef DBG
  TARGETROOT := $(PROJDIR)/bind
else
  TARGETROOT := $(PROJDIR)/bin
endif

# Compiler flags

LIBS = luacom$(LUA_VERSION_NUMBER)

SRCDIR = bin

SRC = luacom_console.cpp








