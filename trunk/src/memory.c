/*	scelib - Simple C Extension Library
 *  Copyright (C) 2005-2007 Richard 'riri' GILL <richard@houbathecat.info>
 *
 *  memory.c - memory handling functions.
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

#include "scelib/memory.h"
#include <string.h>
#include <errno.h>

/* ========================================================================= */
/* Public functions                                                          */

/* ------------------------------------------------------------------------- */
/* mem_free()                                                                */

void mem_free(void **pmem) {

	if (pmem) {
		if (*pmem) {
			free(*pmem);
			*pmem = NULL;
		}
	}
	else {
		errno = EINVAL;
	}
}

/* ------------------------------------------------------------------------- */
/* mem_realloc()                                                             */

void *mem_realloc(void **pmem, size_t count) {

	void *ptr, *reptr;

	if (!count) {
		mem_free(pmem);
		return NULL;
	}

	ptr = (pmem) ? *pmem : NULL;
	reptr = (ptr) ? realloc(ptr, count) : calloc(1, count);
	if (!reptr) {
		free(ptr);
		errno = ENOMEM;
	}
	mem_init(pmem, (reptr) ? reptr : NULL);
	return reptr;
}

/* ------------------------------------------------------------------------- */
/* mem_dup()                                                                 */

void *mem_dup(void *mem, size_t size) {

	void *ptr;

	if (!size || !mem) {
		return NULL;
	}

	ptr = malloc(size);
	if (!ptr) {
		return NULL;
	}

	return memcpy(ptr, mem, size);

}

/* vi:set ts=4 sw=4: */
