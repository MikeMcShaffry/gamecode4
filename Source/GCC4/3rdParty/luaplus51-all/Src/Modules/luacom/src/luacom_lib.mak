#
# tecmake file for LuaCOM
#


include luacom.inc

CPPFLAGS = /GR
LIBNAME  = luacom$(LUA_VERSION_NUMBER)

ifeq "$(TEC_UNAME)" "dll"
USE_DLL=YES
endif


# Sources


DEF_FILE = library/luacom.def

ifeq "$(LUA_VERSION_NUMBER)" "5"
SRCLUA = library/luacom5.lua
endif

INCLUDES += .



SRC = LuaAux.cpp \
           luabeans.cpp \
           luacom.cpp \
           tLuaCOM.cpp \
           tLuaCOMException.cpp \
           tLuaCOMTypeHandler.cpp \
           tLuaDispatch.cpp \
           tLuaObjList.cpp \
           tLuaVector.cpp \
           tStringBuffer.cpp \
           tUtil.cpp \
           tCOMUtil.cpp \
           tLuaCOMClassFactory.cpp \
           tLuaCOMConnPoints.cpp \
           LuaCompat.c \
           tLuaCOMEnumerator.cpp \
           tLuaObject.cpp \
           tLuaTLB.cpp

SRC := $(addprefix library/, $(SRC))
