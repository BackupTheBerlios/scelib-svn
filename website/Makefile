# Makefile for scelib web site generation

# tester le generic pattern matching:
# htdocs/index.fr.html: %.fr.html: %.xml
# 
OUTDIR = htdocs
OUTLANGS = fr en

all: $(OUTDIR)/index.fr.html
	echo "pouet"

clean: FORCE
	# remove generated temporary .mak files
	-rm *.mak

src.mak:
	# find all xml source files
	find . -name '*.xml' -not -name 'menu.xml' > src.mak

out.mak: src.mak
	# generate needed output files, according to defined languages
	if [ -e $@ ]; then rm $@ ; touch $@ ; fi
	for l in $(OUTLANGS); do sed -e "s/xml/$$l.html/g" < $< >> $@; done

	
$(OUTDIR)/index.fr.html: index.xml
	$(XSLTPROC) $(XSLTPARAM) lang fr ../trans/main.xsl $< > $@

# old fashion phony target
FORCE:

