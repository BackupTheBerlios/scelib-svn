# Unix Makefile for SCELib
# scelib - Simple C Extension Library
# Copyright (C) 2005-2007 Richard 'riri' GILL <richard@houbathecat.info>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

# -------------------------------------------------------------------------
# global definitions

.POSIX:

LIBNAME = scelib
OBJS = memory.o cmdline.o vaprint.o str.o thread.o

# should be detected !
LIBEXT = a
SHEXT = so
#LIBEXT = lib
#SHEXT = dll

# -------------------------------------------------------------------------
# options and tools

AR = ar -r
INSTALL = cp -p
RM = rm -f

# -------------------------------------------------------------------------
# main targets

all:: static

static: lib$(LIBNAME).$(LIBEXT)

shared: lib$(LIBNAME).$(SHEXT)

clean::
	$(RM) -f $(OBJS) lib$(LIBNAME).$(LIBEXT) lib$(LIBNAME).$(SHEXT)

lib$(LIBNAME).a: $(OBJS)
	$(RM) $@
	$(AR) $@ $(OBJS)
	ranlib $@

# vi:set ts=4 sw=4 noet:
