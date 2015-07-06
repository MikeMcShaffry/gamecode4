# Documentation Makefile

APP = rst2html.py
CP  = "copy /y"
RM  = del
IDX = ..\README

ALLVAR = APP=$(APP) CP=$(CP) RM=$(RM) IDX=$(IDX)

.PHONY: all clean

all clean:
	cd ..\..\doc && $(MAKE) $(ALLVAR) $@
