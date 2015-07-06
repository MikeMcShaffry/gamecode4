# makefile for rex_posix library

include ../defaults.mak

# === USER SETTINGS ===
# ===========================================================================

# These are default values.
INC =
LIB =

# If the default settings don't work for your system,
# try to uncomment and edit the settings below.
#INC =
#LIB = -lc

# WARNING:
#   If you want to use a POSIX regex library that is not the system
#   default, make sure you set both the INC and LIB variables correctly,
#   as if a header file and library are used which do not match, you may
#   well get segmentation faults (or worse).

# The following lines work for the rxspencer library, when installed
# under /usr (note the above warning!)
#INC = -I/usr/include/rxspencer
#LIB = -lrxspencer

# Target name
REGNAME = posix

# ===========================================================================
# === END OF USER SETTINGS ===

OBJ    = lposix.o ../common.o

include ../common.mak

# static POSIX regexp library binding
ar_posix: $(TRG_AR)

# dynamic POSIX regexp library binding
so_posix: $(TRG_SO)

# Dependencies
lposix.o: lposix.c ../common.h ../algo.h
../common.o: ../common.c ../common.h

# (End of Makefile)
