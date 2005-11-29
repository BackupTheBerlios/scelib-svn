# Makefile to install resource files

all: build

.POSIX:

include ../plat.mk

build:

clean:

install:
	install -d ../$(OUTDIR)
	install -m 644 index.php ../$(OUTDIR)

uninstall:
	rm -f ../$(OUTDIR)/index.php

