# Project: rex_tre

# User Settings ------------------------------------------------------------
# path of TRE include files
REGEXINC = s:\progr\work\system\include
# --------------------------------------------------------------------------

PROJECT  = rex_tre
MYINCS   = -I$(REGEXINC) -I$(LUAINC) 
MYLIBS   = -L$(DLLPATH) -ltre -l$(LUADLL)
OBJ      = ltre.o common.o
MYCFLAGS = -W -Wall -O2
EXPORTED = 'luaopen_$(PROJECT)'
SRCPATH  = ..\..\src;..\..\src\tre
TESTPATH = ..\..\test
TESTNAME = tre

include _mingw.mak

ltre.o    : common.h algo.h
common.o  : common.h
