/*	scelib - Simple C Extension Library
 *  Copyright (C) 2005-2007 Richard 'riri' GILL <richard@houbathecat.info>
 *
 *  str.h - string handling declarations.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __SCELIB_STR_H
#define __SCELIB_STR_H

#include "defs.h"
#include <stdarg.h>
#include <stdlib.h>

SCELIB_BEGIN_CDECL

char *str_dup(const char *str);
char *str_set(char **dest, size_t *dlen, const char *fmt, ...);
char *str_vset(char **dest, size_t *dlen, const char *fmt, va_list ap);
char *str_grow(char **str, size_t add);
char *str_expand(char **str, size_t pos, size_t count);
char *str_contract(char **str, size_t pos, size_t count);
char *str_adjust(char **str, size_t pos, size_t count, size_t length);

SCELIB_END_CDECL

#endif /* __SCELIB_STR_H */
/* vi:set ts=4 sw=4: */
