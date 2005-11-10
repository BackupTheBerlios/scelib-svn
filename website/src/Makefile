
OUTDIR = htdocs

OUTLANGS = fr en

XSLT = xsltproc
LANGPRM = --stringparam lang

TARGETS = $(OUTDIR)/index.fr.html $(OUTDIR)/index.en.html \
	$(OUTDIR)/legal.fr.html $(OUTDIR)/legal.en.html

all: $(TARGETS)

clean: FORCE
	-rm -f $(OUTDIR)/*.html

$(OUTDIR)/index.fr.html: index.xml
	$(XSLT) $(LANGPRM) fr main.xsl $< > $@

$(OUTDIR)/index.en.html: index.xml
	$(XSLT) $(LANGPRM) en main.xsl $< > $@

$(OUTDIR)/legal.fr.html: legal.xml
	$(XSLT) $(LANGPRM) fr main.xsl $< > $@

$(OUTDIR)/legal.en.html: legal.xml
	$(XSLT) $(LANGPRM) en main.xsl $< > $@

FORCE:

