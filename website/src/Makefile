# Makefile for source xml files
# The major part of the make rules are in the auto-generated Makefile.rules
# in the top directory.

include ../Makefile.rules

XSLT = xsltproc
LANGPRM = --stringparam lang

all: build
	
build: $(TARGETS)

clean: FORCE
	-rm -f $(OUTDIR)/*.html

# FORCE is a special target reproducing .PHONY for old make tools which
# don't support that
FORCE:


