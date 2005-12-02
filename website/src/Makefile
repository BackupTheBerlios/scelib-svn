# Makefile to build html files from xml one

TARGETS = home.fr.html home.en.html \
	about.fr.html about.en.html \
	features.fr.html features.en.html \
	legal.fr.html legal.en.html

all: build

.POSIX:

include ../plat.mk

build: $(TARGETS)

clean:
	-rm -f $(TARGETS)

install: build
	$(MAKE) DESTDIR=../$(OUTDIR) publish

uninstall:
	for f in $(TARGETS); do \
	  rm -f ../$(OUTDIR)/$$f ; \
	done

features.%.html: features.%.xml status.xml shared.xml ../$(PROCDIR)/main.xsl
	$(XSLTPROC) $(XSLTFLAGS) ../$(PROCDIR)/main.xsl $< > $@ || rm -f $@

legal.%.html: legal.%.xml authors.xml shared.xml ../$(PROCDIR)/main.xsl
	$(XSLTPROC) $(XSLTFLAGS) ../$(PROCDIR)/main.xsl $< > $@ || rm -f $@

%.html: %.xml menu.xml shared.xml ../$(PROCDIR)/main.xsl
	$(XSLTPROC) $(XSLTFLAGS) ../$(PROCDIR)/main.xsl $< > $@ || rm -f $@

publish:
	-install -d $(DESTDIR)
	for f in *.html; do \
	  [ $$f -nt $(DESTDIR)/$$f ] && { \
	    echo ">> $$f" ; \
	    cp $$f $(DESTDIR)/$$f ; \
	    $(TIDY) $(TIDYFLAGS) $$f ; \
	  } ; \
	done

