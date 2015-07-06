# Makefile for lrexlib

MKFILES = \
  rex_pcre.mak    \
  rex_spencer.mak \
  rex_onig.mak    \
  rex_gnu.mak     \
  rex_tre.mak

LOOP = @for %%d in ($(MKFILES)) do $(MAKE) -f %%d

all: build test

build:
	$(LOOP)

test:
	$(LOOP) test

install:
	$(LOOP) install

clean:
	$(LOOP) clean

.PHONY: all build test install clean
