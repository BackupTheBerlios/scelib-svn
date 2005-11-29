#!/bin/sh

[ $# -ne 2 ] && {
  echo "usage: $0 <source dir> <output dir>"
  exit 1
}

srcdir=$1
outdir=$2

srcfiles="`find $srcdir -name '*.xml' -not -name 'menu.xml' | sed -e \"s/$srcdir\///g\"`"

# nice header for the Makefile.tmp
cat > Makefile.tmp << EOT
# Makefile.rules: auto-generated rules for all source xml files found in the
# $srcdir directory.
#
# Do not modify, this file will be overwritten

OUTDIR = $outdir

TARGETS =

EOT

for file in $srcfiles; do
  outfile="$outdir/`echo $file | sed -e \"s/xml/html/\"`"
  targets="$targets $outfile"
  echo "$outfile: $srcdir/$file" >> Makefile.tmp
  echo >> Makefile.tmp
done
echo "# generic rule to make html from xml" >> Makefile.tmp
echo >> Makefile.tmp
echo "$outdir/%.html: $srcdir/%.xml" >> Makefile.tmp
echo -e "\t\$(XSLTPROC) \$(PROCDIR)/main.xsl $< > \$@" >> Makefile.tmp
echo >> Makefile.tmp

sed -e "s@TARGETS =@TARGETS = $targets@" < Makefile.tmp > Makefile.rules
rm Makefile.tmp

