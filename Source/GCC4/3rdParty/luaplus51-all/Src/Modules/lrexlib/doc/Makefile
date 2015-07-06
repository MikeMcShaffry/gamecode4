# Documentation Makefile

APP = rst2html
CP  = cp -a
RM  = rm
IDX = ../README

.SUFFIXES: .txt .html

CSS = --stylesheet-path=lrexlib.css --link-stylesheet
HDR = --initial-header-level=2
DT  = --date --time

PAGES = index.html manual.html

.txt.html:
	$(APP) $(CSS) $(HDR) $(DT) $< $@

all: $(PAGES)

index.txt: $(IDX)
	$(CP) $< $@

clean:
	$(RM) $(PAGES) index.txt
