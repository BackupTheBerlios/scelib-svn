# Makefile to build html files from xml one

TARGETS = home.fr.html home.en.html \
	about.fr.html about.en.html \
	news.fr.html news.en.html \
	features.fr.html features.en.html \
	legal.fr.html legal.en.html \
	prj.fr.html prj.en.html

all: build

.POSIX:

include ../plat.mk

build: $(TARGETS)

clean:
	-rm -f $(TARGETS)

install: build
	$(MAKE) DESTDIR=../$(OUTDIR) publish

tidy_install:
	$(MAKE) DESTDIR=../$(OUTDIR) tidy_publish

uninstall:
	for f in $(TARGETS); do \
	  rm -f ../$(OUTDIR)/$$f ; \
	done

features.%.html: features.%.xml status.xml shared.xml ../$(PROCDIR)/main.xsl
	$(XSLTPROC) $(XSLTFLAGS) ../$(PROCDIR)/main.xsl $< > $@ || rm -f $@

legal.%.html: legal.%.xml authors.xml shared.xml ../$(PROCDIR)/main.xsl
	$(XSLTPROC) $(XSLTFLAGS) ../$(PROCDIR)/main.xsl $< > $@ || rm -f $@

news.%.html: news.%.xml newsdb.xml shared.xml ../$(PROCDIR)/main.xsl
	$(XSLTPROC) $(XSLTFLAGS) ../$(PROCDIR)/main.xsl $< > $@ || rm -f $@

%.html: %.xml menu.xml shared.xml ../$(PROCDIR)/main.xsl
	$(XSLTPROC) $(XSLTFLAGS) ../$(PROCDIR)/main.xsl $< > $@ || rm -f $@

publish:
	-install -d $(DESTDIR)
	for f in *.html; do \
	  : ; \
	  [ $$f -nt $(DESTDIR)/$$f ] && { \
	    cp $$f $(DESTDIR)/$$f || break ; \
	  } ; \
	  : ; \
	done

tidy_publish: publish
	find $(DESTDIR) -name '*.html' -exec $(TIDY) $(TIDYFLAGS) {} \;

#	for f in *.html; do \
#	  : ; \
#	  [ $$f -nt $(DESTDIR)/$$f ] && { \
#	    cp $$f $(DESTDIR)/$$f && \
#	    $(TIDY) $(TIDYFLAGS) $$f ; \
#	    [ $$? -ne 0 ] && echo ">> $$f ($$?)" ; \
#	  } ; \
#	  : ; \
#	done
