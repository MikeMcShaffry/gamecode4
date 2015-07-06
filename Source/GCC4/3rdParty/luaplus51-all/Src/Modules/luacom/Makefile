# 
# 
# Makefile for LuaCOM
#
# This makefile is suitable for generating libs
# for Lua 4 and 5 and for debug and release versions
#

# ---------------------------------------------------------------------------
# Version Information
# You must supply the Lua version being used, at least as an environment
# variable. Currently only 4 and 5 are acceptable options
# ---------------------------------------------------------------------------
LUA_SUFFIX=5.1        # suffix for binaries (e.g. 5.1 => lua5.1.exe)
LUACOM_VERSION=1.4

# ---------------------------------------------------------------------------
# Use the following lines to set your installation directories
# ---------------------------------------------------------------------------
INC_DIR = .\include
LIB_DIR = .\lib
OBJ_ROOTDIR = .\obj
OBJ_DIR = $(OBJ_ROOTDIR)\lua$(LUA_SUFFIX)
BIN_DIR = .\bin
DLL_DIR = .\dll
SRC_LIB_DIR = .\src\library
SRC_BIN_DIR = .\src\bin
SRC_DLL_DIR = .\src\dll

# ---------------------------------------------------------------------------
# Use the following lines to set the directories where Lua is installed and
# the pathnames for the tools luac and bin2c. You may also define them in the
# environment.
# WARNING: The version of Lua in these # directories MUST MATCH the one
# specified in LUA_VERSION
# ---------------------------------------------------------------------------
LUA_INC = ..\lua5.1\include
LUA_LIB = ..\lua5.1\lib\static
LUA_LIBDLL = ..\lua5.1\lib
LUA = ..\lua5.1\bin\lua$(LUA_SUFFIX).exe
LUAC = ..\lua5.1\bin\luac$(LUA_SUFFIX).exe
BIN2C = ..\lua5.1\bin\bin2c$(LUA_SUFFIX).exe

# ---------------------------------------------------------------------------
# Use the following lines to ste the IUP, CD, and IM include and library
# directories (only if you will use IUP)
# ---------------------------------------------------------------------------
IUP_INC = y:\trab\iup\include
IUP_LIB = y:\trab\iup\lib
CD_INC = y:\trab\cd\include
CD_LIB = y:\trab\cd\lib
IM_LIB = y:\trab\im\lib
TOLUA_LIB = y:\trab\tolua-5.0\lib

# ---------------------------------------------------------------------------
# Use the new Lua package proposal (http://www.keplerproject.org/compat)
# Comment the following line if your Lua is not compiled with compat-5.1
# Ignored if you are compiling for Lua 4
# ---------------------------------------------------------------------------
COMPAT = -DCOMPAT51

# ---------------------------------------------------------------------------
# Compiler options
# ---------------------------------------------------------------------------
CC = cl

INCS = -I$(LUA_INC) -I$(INC_DIR) -I$(SRC_LIB_DIR)

CFLAGS = $(INCS) -DWIN32 -D_CRT_SECURE_NO_DEPRECATE -nologo

# If you want debugging information, define DEBUG in your environment

!IFDEF DEBUG
CFLAGS =$(CFLAGS) -MDd -Zi -Od -DLUA_USE_APICHECK
!ELSE
CFLAGS =$(CFLAGS) -MD -Zi -O2 -DNDEBUG
!ENDIF

# If you want IUP compiled in, define IUP

!IFDEF IUP
CFLAGS =$(CFLAGS) -DIUP -I$(IUP_INC) -I$(CD_INC)
IUP_LIBS = iup.lib comctl32.lib iupcontrols.lib iupgl.lib cd.lib cdiup.lib iuplua5.lib opengl32.lib glu32.lib im.lib cdlua5.lib tolua.lib Comdlg32.lib
!ELSE
IUP_LIBS =
!ENDIF

CPPFLAGS = /EHsc $(CFLAGS)

.SUFFIXES:
.SUFFIXES: .c .cpp .obj

# ---------------------------------------------------------------------------
# Linker options
# ---------------------------------------------------------------------------
DEFAULT_LIBS = kernel32.lib user32.lib gdi32.lib shell32.lib advapi32.lib ole32.lib winspool.lib uuid.lib oleaut32.lib shlwapi.lib htmlhelp.lib
#				/NODEFAULTLIB:"libcd.lib" /NODEFAULTLIB:"msvcrt.lib" \
#				/NODEFAULTLIB:"msvcrtd.lib" /NODEFAULTLIB:"libcmt.lib" \
#				/NODEFAULTLIB:"libcmtd.lib" \


LFLAGS = /nologo /subsystem:console /incremental:no /machine:I386 /debug \
				$(DEFAULT_LIBS) 

!IFDEF DEBUG
LINK_USE_DEBUG = /DEBUG /PDB:NONE
!ENDIF

# Here we assume lua libs are have as a suffix the version number, like lua-5.0.lib
LUA_LIBS = $(LUA_LIB)/lua$(LUA_SUFFIX).lib
LUA_LIBSDLL = $(LUA_LIBDLL)/lua$(LUA_SUFFIX).lib 

# ---------------------------------------------------------------------------
# Target options
# ---------------------------------------------------------------------------
LC_LIB = $(LIB_DIR)\luacom-lua$(LUA_SUFFIX)-$(LUACOM_VERSION).lib
LC_DLL = $(BIN_DIR)\luacom.dll
LC_IMPLIB = $(LIB_DIR)\luacomdll-lua$(LUA_SUFFIX)-$(LUACOM_VERSION).lib

LC_CONSOLE_LIB = $(BIN_DIR)\luacom-lua$(LUA_SUFFIX)-$(LUACOM_VERSION).exe
LC_CONSOLE_DLL = $(BIN_DIR)\luacomdll-lua$(LUA_SUFFIX)-$(LUACOM_VERSION).exe

all: lc_lib lc_dll lc_bin_lib lc_bin_dll
lc_lib: $(LIB_DIR) $(OBJ_DIR) $(LC_LIB)
lc_dll: $(BIN_DIR) $(OBJ_DIR) $(LC_DLL)
lc_bin_lib: $(OBJ_DIR) $(BIN_DIR) $(LC_CONSOLE_LIB)
lc_bin_dll: $(OBJ_DIR) $(BIN_DIR) $(LC_CONSOLE_DLL) $(BIN_DIR)/luacom.lua

$(LIB_DIR)::
	if not exist $(LIB_DIR) \
	   mkdir $(LIB_DIR)

$(OBJ_DIR)::
	if not exist $(OBJ_DIR) \
	   mkdir $(OBJ_DIR)

$(BIN_DIR)::
	if not exist $(BIN_DIR) \
	   mkdir $(BIN_DIR)

install:
	if not exist "$(LUA_LIBDIR)" mkdir -p "$(LUA_LIBDIR)"
	copy $(BINDIR)\luacom.dll "$(LUA_LIBDIR)"

clean-obj:
	if exist $(OBJ_DIR)\*.obj del $(OBJ_DIR)\*.obj 
	if exist $(OBJ_DIR) rmdir $(OBJ_DIR)

clean:
	if exist $(OBJ_ROOTDIR) rmdir /S /Q $(OBJ_ROOTDIR)
	if exist $(LIB_DIR)\*.lib del $(LIB_DIR)\*.lib
	if exist $(LIB_DIR)\*.exp del $(LIB_DIR)\*.exp
	if exist $(LIB_DIR) rmdir $(LIB_DIR)
	if exist $(BIN_DIR)\*.exe del $(BIN_DIR)\*.exe
	if exist $(BIN_DIR)\*.dll del $(BIN_DIR)\*.dll
	if exist $(BIN_DIR)\*.manifest del $(BIN_DIR)\*.manifest
	if exist $(BIN_DIR)\*.pdb del $(BIN_DIR)\*.pdb
	if exist $(BIN_DIR)\luacom.lua del $(BIN_DIR)\luacom.lua
	if exist $(BIN_DIR) rmdir $(BIN_DIR)

DEF = $(SRC_DLL_DIR)/luacom_dll.def

LIB_OBJS = $(OBJ_DIR)/LuaAux.obj \
           $(OBJ_DIR)/luabeans.obj \
           $(OBJ_DIR)/luacom.obj \
           $(OBJ_DIR)/tLuaCOM.obj \
           $(OBJ_DIR)/tLuaCOMException.obj \
           $(OBJ_DIR)/tLuaCOMTypeHandler.obj \
           $(OBJ_DIR)/tLuaDispatch.obj \
           $(OBJ_DIR)/tLuaObjList.obj \
           $(OBJ_DIR)/tLuaVector.obj \
           $(OBJ_DIR)/tStringBuffer.obj \
           $(OBJ_DIR)/tUtil.obj \
           $(OBJ_DIR)/tCOMUtil.obj \
           $(OBJ_DIR)/tLuaCOMClassFactory.obj \
           $(OBJ_DIR)/tLuaCOMConnPoints.obj \
           $(OBJ_DIR)/LuaCompat.obj \
           $(OBJ_DIR)/tLuaCOMEnumerator.obj \
           $(OBJ_DIR)/tLuaObject.obj \
           $(OBJ_DIR)/tLuaControl.obj \
           $(OBJ_DIR)/tLuaTLB.obj

BIN_OBJS = $(OBJ_DIR)/luacom_console.obj

DLL_OBJS = $(OBJ_DIR)/luacom_dll.obj

{$(SRC_LIB_DIR)}.cpp{$(OBJ_DIR)}.obj:
	$(CC) /c /DLUACOM_DLL=\"luacom.dll\" $(CPPFLAGS) $(COMPAT) /Fo$@ $<

{$(SRC_LIB_DIR)}.c{$(OBJ_DIR)}.obj:
	$(CC) /c /Za $(CFLAGS) $(COMPAT) /Fo$@ $<

{$(SRC_BIN_DIR)}.cpp{$(OBJ_DIR)}.obj:
	$(CC) /c $(CPPFLAGS) $(COMPAT) /Fo$@ $<

{$(SRC_DLL_DIR)}.cpp{$(OBJ_DIR)}.obj:
	$(CC) /c $(CPPFLAGS) $(COMPAT) /Fo$@ /I$(SRC_DLL_DIR) $<

include ./mak/dependences.mak

$(BIN_DIR)/luacom.lua: $(SRC_LIB_DIR)/t_luacom.lua
	$(LUA) -e"str=io.read('*a');str=string.gsub(str,'LUACOM_DLL','luacom.dll');io.write(str);" < $(SRC_LIB_DIR)/t_luacom.lua > $@

$(LC_LIB): $(LIB_OBJS)
	lib /NOLOGO /OUT:$@ $(LIB_OBJS)

$(LC_CONSOLE_LIB): $(BIN_OBJS) $(LC_LIB)
	link /out:$@ $(BIN_OBJS) $(LC_LIB) $(LFLAGS) $(LUA_LIBS)

$(LC_CONSOLE_DLL): $(BIN_OBJS) $(LC_DLL)
	link /out:$@ $(BIN_OBJS) $(LC_IMPLIB) $(LFLAGS) $(LUA_LIBSDLL)

$(LC_DLL): $(LIB_OBJS) $(DLL_OBJS) $(DEF)
	link /DLL /IMPLIB:$(LC_IMPLIB) /LIBPATH:$(IUP_LIB) /LIBPATH:$(IM_LIB) /LIBPATH:$(CD_LIB) /LIBPATH:$(TOLUA_LIB) /out:$@ /DEF:$(DEF) $(LIB_OBJS) $(DLL_OBJS) $(LFLAGS) $(LUA_LIBSDLL) $(IUP_LIBS)

.SUFFIXES: .lua

{$(SRC_LIB_DIR)}.lua{$(SRC_LIB_DIR)}.loh:
	$(LUAC) -o $*.lo $<
	$(BIN2C) $< > $@	
	del $*.lo


