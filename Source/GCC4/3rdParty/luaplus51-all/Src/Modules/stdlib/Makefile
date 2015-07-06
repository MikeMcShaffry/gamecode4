all:
	@echo "REL=VERSION make {release,dist}"

dist:
	cd modules && ../utils/ldoc *.lua
	cd .. && tar czf stdlib-${REL}.tar.gz --exclude=CVS --exclude=.cvsignore --exclude=".#*" --exclude="release-notes-*" stdlib

release: dist
	cvs diff && \
	cvs tag rel-${REL} && \
	woger lua-l stdlib stdlib "release ${REL}" "General Lua libraries" release-notes-${REL}
