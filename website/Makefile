# Makefile for html output

SRCDIR = src
PROCDIR = proc
OUTDIR = htdocs
RESDIR = res

XSLTPROC = xsltproc
XSLTFLAGS =  -nonet
TIDY = tidy
TIDYFLAGS = -asxml -i -utf8 -m -n

all: help

help:
	@echo "make {website|build|clean|update|help]"
	@echo " * website will copy all necessary files to output"
	@echo " * build will rebuild the website"
	@echo " * clean will delete all generated files"
	@echo " * update will update Makefile.rules to all found source files"
	@echo " * help will output this notice"

-include Makefile.rules

website: build
	cp -R $(RESDIR)/* $(OUTDIR)

build: update
	make $(TARGETS)

clean:
	-rm -f $(OUTDIR)/*.html
	-rm -f Makefile.rules

update:
	. $(PROCDIR)/update-rules.sh $(SRCDIR) $(OUTDIR)

$(OUTDIR)/%.html: $(SRCDIR)/%.xml
	$(XSLTPROC) $(XSLTFLAGS) $(PROCDIR)/main.xsl $< > $@
	$(TIDY) $(TIDYFLAGS) $@ 2>/dev/null

# FORCE is a psecial target reproducing .PHONY for old tools which don't
# support that
FORCE:
