#
# luacom.inc
#
#   Common definitions for building LuaCOM and luacom_console
#   using tecmake
#


PROJNAME = luacom

PROJDIR=../tecgraf

# Assigns default values to variables

APPTYPE=console

ifeq "$(DBG)" "NO"
DBG=
else
DBG=YES
endif

ifndef LUA_VERSION_NUMBER
LUA_VERSION_NUMBER=5
endif

ifndef TEC_UNAME
TEC_UNAME = vc6
endif

USE_NODEPEND=YES

# Forces use of local compilation

ifndef TEC_NOLOCAL
VC6=C:\\APP\\LNG\\msdev6\\vc
VC7=C:\\APP\\LNG\\vc7
endif
OBJROOT=c:\\temp\\$(PROJNAME)\\obj




# Fixes library directory

ifdef DBG
LIBDIR:=$(PROJDIR)\\libd
else
LIBDIR:=$(PROJDIR)\\lib
endif

#  Compiler flags

INCLUDES = ..\\include
DEFINES += LUA$(LUA_VERSION_NUMBER)

ifdef DBG
OBJROOT:=$(OBJROOT)d
DEFINES += VERBOSE
endif


USE_LUA5=YES

LDIR = $(LIBDIR)\\$(TEC_UNAME)

USE_DLL=
