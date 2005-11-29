# Makefile for html output

SRCDIR = src
PROCDIR = proc
OUTDIR = htdocs
RESDIR = res

XSLTPROC = xsltproc -nonet

-include Makefile.rules

all: help

help:
	@echo "make {build|clean|update|help]"
	@echo " default target is 'build'"
	@echo " * build will rebuild the website"
	@echo " * clean will delete all generated files"
	@echo " * update will update Makefile.rules to all found source files"
	@echo " * help will output this notice"

build: update
	make $(TARGETS)

clean:
	-rm -f $(OUTDIR)/*.html
	-rm -f Makefile.rules

#Makefile.rules: update

update:
	. $(PROCDIR)/update-rules.sh $(SRCDIR) $(OUTDIR)

# FORCE is a psecial target reproducing .PHONY for old tools which don't
# support that
FORCE:

