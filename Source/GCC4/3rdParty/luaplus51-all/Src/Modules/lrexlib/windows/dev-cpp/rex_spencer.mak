# Project: rex_spencer

# User Settings ------------------------------------------------------------
# path of Spencer's include files
REGEXINC = s:\progr\work\system\include\rxspencer
# --------------------------------------------------------------------------

PROJECT  = rex_spencer
MYINCS   = -I$(REGEXINC) -I$(LUAINC) 
MYLIBS   = -L$(DLLPATH) -lrxspencer -l$(LUADLL)
OBJ      = lposix.o common.o
MYCFLAGS = -W -Wall -O2
EXPORTED = 'luaopen_$(PROJECT)'
SRCPATH  = ..\..\src;..\..\src\posix
TESTPATH = ..\..\test
TESTNAME = spencer

include _mingw.mak

lposix.o  : common.h algo.h
common.o  : common.h
