# Makefile to build html files from xml one

TARGETS = home.fr.html home.en.html \
	  legal.fr.html legal.en.html \
	  features.fr.html

all: build

.POSIX:

include ../plat.mk

build: $(TARGETS)

clean:
	-rm -f $(TARGETS)

install: build
	install -d ../$(OUTDIR)
	for f in *.html; do \
	  [ $$f -nt ../$(OUTDIR)/$$f ] && { \
	    cp $$f ../$(OUTDIR)/$$f ; \
	    $(TIDY) $(TIDYFLAGS) $$f ; \
	  } ; \
	done

uninstall:
	for f in $(TARGETS); do \
	  rm -f ../$(OUTDIR)/$$f ; \
	done

features.%.html: features.%.xml status.xml ../$(PROCDIR)/main.xsl
#	$(XSLTPROC) $(XSLTFLAGS) ../$(PROCDIR)/main.xsl $< > $@ ; \
#	if [ $$? -ne 0 ]; then \
#	  rm -f $@ ; \
#	else \
#	  $(TIDY) $(TIDYFLAGS) $@ ; \
#	fi
	$(XSLTPROC) $(XSLTFLAGS) ../$(PROCDIR)/main.xsl $< > $@ || rm -f $@

%.html: %.xml menu.xml ../$(PROCDIR)/main.xsl
	$(XSLTPROC) $(XSLTFLAGS) ../$(PROCDIR)/main.xsl $< > $@ || rm -f $@


