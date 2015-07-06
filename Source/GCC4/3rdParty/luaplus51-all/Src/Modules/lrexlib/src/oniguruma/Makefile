# makefile for rex_onig library

include ../defaults.mak

# === USER SETTINGS ===
# ===========================================================================

# These are default values.
INC =
LIB = -lonig

# If the default settings don't work for your system,
# try to uncomment and edit the settings below.
#INC =
#LIB = -lonig

# Target name
REGNAME = onig

# ===========================================================================
# === END OF USER SETTINGS ===

OBJ    = lonig.o lonig_f.o ../common.o

include ../common.mak

# static Oniguruma regexp library binding
ar_onig: $(TRG_AR)

# dynamic Oniguruma regexp library binding
so_onig: $(TRG_SO)

# Dependencies
lonig.o: lonig.c ../common.h ../algo.h
lonig_f.o: lonig_f.c ../common.h
../common.o: ../common.c ../common.h

# (End of Makefile)
