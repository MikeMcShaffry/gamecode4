# makefile for rex_pcre library

include ../defaults.mak

# === USER SETTINGS ===
# ===========================================================================

# These are default values.
INC =
LIB = -lpcre

# If the default settings don't work for your system,
# try to uncomment and edit the settings below.
#INC =
#LIB = -lpcre

# Target name
REGNAME = pcre

# ===========================================================================
# === END OF USER SETTINGS ===

OBJ    = lpcre.o lpcre_f.o ../common.o

include ../common.mak

# static PCRE regexp library binding
ar_pcre: $(TRG_AR)

# dynamic PCRE regexp library binding
so_pcre: $(TRG_SO)

# Dependencies
lpcre.o: lpcre.c ../common.h ../algo.h
lpcre_f.o: lpcre_f.c ../common.h
../common.o: ../common.c ../common.h

# (End of Makefile)
