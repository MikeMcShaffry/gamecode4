# makefile for rex_gnu library

include ../defaults.mak

# === USER SETTINGS ===
# ===========================================================================

# These are default values.
INC =
LIB =

# If the default settings don't work for your system,
# try to uncomment and edit the settings below.
# The default settings below work with a libgnu.a in the current directory,
# copied from any libgnu.a built with the regex module.
#INC = -I. -DREX_GNU_INCLUDE='"regex.h"'
#LIB = -L. -lgnu

# Regex library name
REGNAME = gnu

# ===========================================================================
# === END OF USER SETTINGS ===

OBJ    = lgnu.o ../common.o

include ../common.mak

# static GNU regexp library binding
ar_gnu: $(TRG_AR)

# dynamic GNU regexp library binding
so_gnu: $(TRG_SO)

# Dependencies
lgnu.o: lgnu.c ../common.h ../algo.h
../common.o: ../common.c ../common.h

# (End of Makefile)
