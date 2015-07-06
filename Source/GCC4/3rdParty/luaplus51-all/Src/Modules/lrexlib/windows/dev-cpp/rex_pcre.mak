# Project: rex_pcre

# User Settings ------------------------------------------------------------
# path of PCRE include files
REGEXINC = s:\progr\work\system\include
# --------------------------------------------------------------------------

PROJECT  = rex_pcre
MYINCS   = -I$(REGEXINC) -I$(LUAINC) 
MYLIBS   = -L$(DLLPATH) -lpcre -l$(LUADLL)
OBJ      = lpcre.o lpcre_f.o common.o
MYCFLAGS = -W -Wall -O2
EXPORTED = 'luaopen_$(PROJECT)'
SRCPATH  = ..\..\src;..\..\src\pcre
TESTPATH = ..\..\test
TESTNAME = pcre

include _mingw.mak

lpcre.o   : common.h algo.h
lpcre_f.o : common.h
common.o  : common.h
