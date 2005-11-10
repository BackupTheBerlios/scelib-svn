
SRCDIR = src
OUTDIR = htdocs

all: help

build: Makefile.rules
	make -C $(SRCDIR) $@

clean:
	make -C $(SRCDIR) $@
	-rm Makefile.rules

help:
	@echo "make [build|clean|update|help]"
	@echo "  default target is 'build'"
	@echo "  * make build will rebuild the website"
	@echo "  * make clean will delete all generated files"
	@echo "  * make update will update Makefile.rules to all found xml files"
	@echo "  * make help will output this notice"

# redirection to be able to call 'make update' to force Makefile.rules
# regeneration
Makefile.rules:
	make update

update:
	. ./update-rules.sh $(SRCDIR) $(OUTDIR)

