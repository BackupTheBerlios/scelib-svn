# Makefile to install resource files

all: build

.POSIX:

include ../plat.mk

build:

clean:

install:
	$(MAKE) DESTDIR=../$(OUTDIR) publish

tidy_install:

uninstall:
	rm -f ../$(OUTDIR)/index.php

publish:
	-install -d $(DESTDIR)
	install -m 644 index.php $(DESTDIR)
	
tidy_publish:
