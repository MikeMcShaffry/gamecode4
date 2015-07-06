# tested with GNU Make

# User Settings ------------------------------------------------------------
# path of Lua include files
LUAINC = s:\progr\work\system\include

# path of Lua DLL and regexp library DLL
DLLPATH = c:\exe

# name of Lua DLL to link to (.dll should be omitted)
LUADLL = lua5.1

# path to install rex_onig.dll
INSTALLPATH = s:\exe\lib\lua\5.1
# --------------------------------------------------------------------------

LIBS       = $(MYLIBS) -s
INCS       = $(MYINCS)
BIN        = $(PROJECT).dll
DEFFILE    = $(PROJECT).def
BININSTALL = $(INSTALLPATH)\$(BIN)
CC         = gcc.exe
CFLAGS     = $(INCS) -DREX_OPENLIB=luaopen_$(PROJECT) \
  -DREX_LIBNAME=\"$(PROJECT)\" $(MYCFLAGS)

.PHONY: all install test clean

vpath %.c $(SRCPATH)
vpath %.h $(SRCPATH)

all: $(BIN)

clean:
	del $(OBJ) $(BIN) $(DEFFILE)

install: $(BININSTALL)

test:
	cd $(TESTPATH) && lua runtest.lua $(TESTNAME) -d$(CURDIR)

$(BIN): $(OBJ) $(DEFFILE)
	$(CC) $(DEFFILE) $(OBJ) $(LIBS) -o $@ -shared

$(DEFFILE):
	lua -e"print('EXPORTS') for k,v in ipairs{$(EXPORTED)} do \
      print('\t'..v) end" > $@

$(BININSTALL): $(BIN)
	copy /Y $< $@
