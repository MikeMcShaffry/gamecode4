# Makefile for lrexlib

# See src/*/Makefile and src/defaults.mak for user-definable settings

include src/defaults.mak

REGNAMES = gnu pcre posix oniguruma tre
PROJECT = lrexlib
VERSION = $(V).$(MINORV)
PROJECT_VERSIONED = $(PROJECT)-$(VERSION)
DISTFILE = $(PROJECT_VERSIONED).zip

all:
	@for i in $(REGNAMES); do \
	  make -C src/$$i; \
	done
	@make -C doc

check: all
	@for i in $(REGNAMES); do \
	  make -C src/$$i check; \
	done

clean:
	@for i in $(REGNAMES); do \
	  make -C src/$$i clean; \
	done
	@make -C doc clean

dist: all
	git2cl > ChangeLog
	cd .. && rm -f $(DISTFILE) && zip $(DISTFILE) -r $(PROJECT) -x "lrexlib/.git/*" "*.gitignore" "*.o" "*.a" "*.so" "*.so.*" "*.zip" "*SciTE.properties" "*scite.properties" && mv $(DISTFILE) $(PROJECT) && cd $(PROJECT) && unzip $(DISTFILE) && mv $(PROJECT) $(PROJECT_VERSIONED) && rm -f $(DISTFILE) && zip $(DISTFILE) -r $(PROJECT_VERSIONED) && rm -rf $(PROJECT_VERSIONED)

release:
	agrep -d 'Release' $(VERSION) NEWS | tail -n +3 | head -n -2 > release-notes && \
	git diff --exit-code && \
	git tag -a -m "Release tag" rel-`echo $(VERSION) | sed -e 's/\./-/g'` && \
	git push && \
	woger lua-l $(PROJECT) "$(PROJECT)" $(VERSION) "Lua binding for regex libraries" release-notes
	@cat release-notes && echo "Don't forget to release on LuaForge!"
	rm -f release-notes
