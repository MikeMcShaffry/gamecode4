VERSION= luajson-$(shell git describe --abbrev=4 HEAD 2>/dev/null)

DIST_TAR= $(VERSION).tar
DIST_BZ2= $(DIST_TAR).bz2
DIST_GZ=  $(DIST_TAR).gz
DIST_ZIP= $(VERSION).zip

.PHONY: all clean dist distclean dist-bzip2 dist-gzip dist-tar dist-zip
.DELETE_ON_ERROR:

all: dist-all

clean: dist-clean
	@echo Cleaning nothing - no binaries

all dist dist-all: dist-tar dist-bzip2 dist-gzip dist-zip

distclean:
	rm -f luajson-*.bz2
	rm -f luajson-*.gz
	rm -f luajson-*.tar
	rm -f luajson-*.zip

dist-tar: $(DIST_TAR)

dist-bzip2: $(DIST_BZ2)

dist-gzip: $(DIST_GZ)

dist-zip: $(DIST_ZIP)

$(DIST_TAR):
	cd .. && git archive --format=tar --prefix=$(VERSION)/ HEAD . -o $(CURDIR)/$@

$(DIST_BZ2): $(DIST_TAR)
	bzip2 -9 -c -f $(DIST_TAR) > $@

$(DIST_GZ): $(DIST_TAR)
	gzip -9 -c -f $(DIST_TAR) > $@

$(DIST_ZIP):
	cd .. && git archive --format=zip --prefix=$(VERSION)/ HEAD: -o $(CURDIR)/$@
