#!/bin/sh

[ $# -ne 2 ] && {
  echo "usage: $0 <source dir> <output dir>"
  exit 1
}

srcdir=$1
outdir=$2

langfile="`grep -v -e '^$' -e '^#.*$' < Languages`"
xmlfiles="`find $srcdir -name '*.xml' -not -name 'menu.xml' | sed -e \"s/$srcdir\///g\"`"

# nice header for the Makefile.tmp"
cat > Makefile.tmp << EOT
# Makefile.rules: auto-generated rules for all source xml files found in the
# $srcdir directory, and all languages defined in the 'Languages' file.
#
# Do not modify, this file will be overwritten

OUTDIR = ../$outdir

TARGETS = 

EOT

for lang in $langfile; do
  echo "## rules for the '$lang' language" >> Makefile.tmp
  echo >> Makefile.tmp
  echo "# rules for each source file" >> Makefile.tmp
  echo >> Makefile.tmp
  for file in $xmlfiles; do
    outfile="../$outdir/`echo $file | sed -e \"s/xml/$lang.html/\"`"
    targets="$targets $outfile"
    echo "$outfile: $file" >> Makefile.tmp
    echo >> Makefile.tmp
  done
  echo "# generic rule to make '$lang' outputs" >> Makefile.tmp
  echo >> Makefile.tmp
  echo "../$outdir/%.$lang.html: %.xml" >> Makefile.tmp
  echo -e "\t\$(XSLT) \$(LANGPRM) $lang main.xsl \$< > \$@" >> Makefile.tmp
  echo >> Makefile.tmp
done

sed -e "s@TARGETS =@TARGETS =$targets@" < Makefile.tmp > Makefile.rules
rm Makefile.tmp

