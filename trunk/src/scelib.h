/*
 *  scelib - Simpliest C Extension Library
 *  Copyright (C) 2005-2006 Richard 'riri' GILL <richard@houbathecat.info>
 *
 *  scelib.h - header file of the library.
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

#ifndef _SCELIB_H
#define _SCELIB_H

/* ========================================================================= */
/* header's header :-)                                                       */

#ifdef __cplusplus
#define _begin_cdecl_ extern "C" {
#define _end_cdecl }
#else
#define _begin_cdecl
#define _end_cdecl
#endif

_begin_cdecl



/* ========================================================================= */
/* memory functions                                                          */

/* ------------------------------------------------------------------------- */
/* scemem_init()                                                             */
/* initialize ptr with val only if ptr is a valid pointer */

#define scemem_init(ptr, val) \
	if (ptr) { *(ptr) = (val); }

/* ------------------------------------------------------------------------- */
/* scemem_new()                                                              */
/* shortcut for memory allocation (with me zeroed) */

#define scemem_new(type, cnt) \
	(type *) calloc(sizeof(type), (cnt))

/* ------------------------------------------------------------------------- */
/* scemem_free()                                                             */
/* free memory like free() do, but also reinit pointer to NULL.              */

void scemem_free(void **pmem);

/* ------------------------------------------------------------------------- */
/* scemem_realloc()                                                          */
/* do the same job that the standard realloc() function, but if it fails,    */
/* automatically free the input buffer pointed by pmem, and initialize the   */
/* pointer to null. Returns the equivalent of *pmem.                         */
/* If count == 0, scemem_realloc() acts like scemem_free().                  */

void *scemem_realloc(void **pmem, size_t count);

/* ------------------------------------------------------------------------- */
/* scemem_renew()                                                            */
/* shortcut to scemem_realloc() with scemem_new() semantics (type spec)      */

#define scemem_renew(mem, type, cnt) \
	(type *) mem_realloc((void **)(&(mem)), sizeof(type) * (cnt))


/* ========================================================================= */
/* header's footer                                                           */

_end_cdecl

#endif	/* _SCELIB_H */
/* vi:set ts=4 sw=4: */
