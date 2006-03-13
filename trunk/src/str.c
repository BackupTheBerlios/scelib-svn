/*
 *  scelib - Simpliest C Extension Library
 *  Copyright (C) 2005-2006 Richard 'riri' GILL <richard@houbathecat.info>
 *
 *  str.c - string functions.
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
 *
 */

#include "scelib.h"
#include <string.h>

/* ------------------------------------------------------------------------- */
/* vaprint()                                                                 */
/* Format the fmt string with variable length of arguments, like printf(),   */
/* and output the resulting string in a new allocated one.                   */
/* If dest is NULL, it's not used (the output is just returned), if dlen is  */
/* NULL, size of resulting string is not returned.                           */
char *vaprint(char **dest, size_t *dlen, const char *fmt, va_list ap);

/* ========================================================================= */
/* Public functions definitions                                              */

/* ------------------------------------------------------------------------- */
/* str_dup()                                                                 */

char *str_dup(const char *str) {

	char *dest;
	size_t len;

	if (!str) {
		errno = EINVAL;
		return NULL;
	}

	if (!(dest = mem_new(char, (len = strlen(str) + 1)))) {
		return NULL;
	}

	return (char *) memcpy(dest, str, len);

}

/* ------------------------------------------------------------------------- */
/* str_set()                                                                 */

char *str_set(char **dest, size_t *dlen, const char *fmt, ...) {

	char *ptr;
	va_list ap;
	va_start(ap, fmt);
	ptr = str_vset(dest, dlen, fmt, ap);
	va_end(ap);
	return ptr;

}

/* ------------------------------------------------------------------------- */
/* str_vset()                                                                */

char *str_vset(char **dest, size_t *dlen, const char *fmt, va_list ap) {

	return vaprint(dest, dlen, fmt, ap);

}

/* ------------------------------------------------------------------------- */
/* str_grow()                                                                */

char *str_grow(char **str, size_t add) {

	if (!str) {
		errno = EINVAL;
		return NULL;
	}
	return mem_renew(*str, char, strlen(*str) + add);

}

/* ------------------------------------------------------------------------- */
/* str_expand()                                                              */
char *str_expand(char **str, size_t pos, size_t count) {

	if (!str_grow(str, count)) {
		return NULL;
	}
	return (char *) memmove(*str + pos + count, *str + pos, (strlen(*str) - pos) * sizeof(char));

}

/* ------------------------------------------------------------------------- */
/* str_contract()                                                            */
char *str_contract(char **str, size_t pos, size_t count) {

	size_t orig = (str) ? strlen(*str) : 0;

	if (!str) {
		errno = EINVAL;
		return NULL;
	}

	memmove(*str + pos, *str + pos + count, (orig - pos - count) * sizeof(char));
	return mem_renew(*str, char, orig - count);

}

/* ------------------------------------------------------------------------- */
/* str_adjust()                                                              */
char *str_adjust(char **str, size_t pos, size_t count, size_t length) {

	if (count < length) {
		return str_expand(str, pos + count, length - count);
	}
	return str_contract(str, pos + length, count - length);

}
