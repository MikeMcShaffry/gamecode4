# makefile for rex_tre library

include ../defaults.mak

# === USER SETTINGS ===
# ===========================================================================

# These are default values.
INC =
LIB = -ltre

# If the default settings don't work for your system,
# try to uncomment and edit the settings below.
#INC = -I/usr/include
#LIB = -lc

# WARNING:
#   Make sure you set both the INC and LIB variables correctly, as
#   otherwise you risk using a header file and library that do not
#   match, and you may well get segmentation faults (or worse).

# Target name
REGNAME = tre

# ===========================================================================
# === END OF USER SETTINGS ===

OBJ    = ltre.o ../common.o

include ../common.mak

# static TRE regexp library binding
ar_tre: $(TRG_AR)

# dynamic TRE regexp library binding
so_tre: $(TRG_SO)

# Dependencies
ltre.o: ltre.c ../common.h ../algo.h
../common.o: ../common.c ../common.h

# (End of Makefile)
