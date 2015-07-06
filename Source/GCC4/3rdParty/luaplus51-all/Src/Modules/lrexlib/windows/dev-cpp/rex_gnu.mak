# Project: rex_gnu

# User Settings ------------------------------------------------------------
# path of GNU include files
REGEXINC = s:\progr\work\system\include\gnuregex
# --------------------------------------------------------------------------

PROJECT  = rex_gnu
MYINCS   = -I$(REGEXINC) -I$(LUAINC) 
MYLIBS   = -L$(DLLPATH) -lregex2 -l$(LUADLL)
OBJ      = lgnu.o common.o
MYCFLAGS = -W -Wall -O2
EXPORTED = 'luaopen_$(PROJECT)'
SRCPATH  = ..\..\src;..\..\src\gnu
TESTPATH = ..\..\test
TESTNAME = gnu

include _mingw.mak

lgnu.o    : common.h algo.h
common.o  : common.h
