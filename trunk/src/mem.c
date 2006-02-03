/*
 *  scelib - Simpliest C Extension Library
 *  Copyright (C) 2005-2006 Richard 'riri' GILL <richard@houbathecat.info>
 *
 *  mem.c - memory functions.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "scelib.h"
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

/* ========================================================================= */
/* vi:set ts=4 sw=4: */
