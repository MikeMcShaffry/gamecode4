
CURRENT_VERSION = 1.3.1
SINGLE_FILE_DIR = luacom-htmldoc
BROWSEABLE_DIR = luacom-browseable

WWW = $(HOME)/public/html/luacom/pub/$(CURRENT_VERSION)
PDF = $(WWW)/luacom-$(CURRENT_VERSION)-doc.pdf

HTML_TGZ = $(WWW)/luacom-$(CURRENT_VERSION)-htmldoc.tgz

all: luacom.pdf $(SINGLE_FILE_DIR)/luacom.html $(BROWSEABLE_DIR)/index.html

luacom.pdf: luacom.tex
	pdflatex luacom.tex
	pdflatex luacom.tex
	pdflatex luacom.tex

$(SINGLE_FILE_DIR)/luacom.html: luacom.tex
	latex2html -dir $(SINGLE_FILE_DIR) -split 0 -show_section_numbers -local_icons -no_navigation -numbered_footnotes luacom.tex

$(BROWSEABLE_DIR)/index.html: luacom.tex
	latex2html -dir $(BROWSEABLE_DIR) -show_section_numbers -local_icons -numbered_footnotes luacom.tex

update-www: all
	\cp -f luacom.pdf $(PDF)
	rm -rf $(WWW)/$(SINGLE_FILE_DIR)/*
	cp -R $(SINGLE_FILE_DIR) $(WWW)/
	rm -rf $(WWW)/$(BROWSEABLE_DIR)
	cp -R $(BROWSEABLE_DIR)  $(WWW)/
	tar -cz -f $(HTML_TGZ) $(SINGLE_FILE_DIR)/luacom.html $(SINGLE_FILE_DIR)/*.png


clean:
	rm -rf luacom.toc
	rm -rf luacom.aux
	rm -rf luacom.log
	rm -rf luacom.out
	rm -rf luacom.dvi
	rm -rf *~

clean-all: clean
	rm -rf $(SINGLE_FILE_DIR)
	rm -rf $(BROWSEABLE_DIR)
	rm -rf luacom.pdf



dir:
	mkdir $(BROWSEABLE_DIR)
	mkdir $(SINGLE_FILE_DIR)



