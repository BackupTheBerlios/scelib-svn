/*	scelib - Simple C Extension Library
 *  Copyright (C) 2005-2007 Richard 'riri' GILL <richard@houbathecat.info>
 *
 *  memory.h - memory handling declarations.
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
/** @file
 *	@brief Memory handling declarations.
 *
 *	C programers usually play with dymanic memory functions, like malloc(),
 *	free(), calloc(), realloc() or others. The memory functions provide a
 *	simplier way to use dynamic memory. Of course, the developer can still use
 *	standard memory functions, there's no side effect on their use.
 */

#ifndef __SCELIB_MEMORY_H
#define __SCELIB_MEMORY_H

#include "defs.h"
#include <stdlib.h>

SCELIB_BEGIN_CDECL

/** Init a pointer if the pointee is valid.
 *
 *	Initialize the pointed variable @a *ptr to @a val only if @a ptr is not
 *	null. With a <em>zero all memory buffers</em> scheme, this's useful to
 *	quickly affect a value without explicitly doing the test each time.
 */
#define mem_init(ptr, val) \
	if (ptr) { *(ptr) = (val); }

/** C++ style heap memory allocation.
 *
 *	Memory allocation is always done with zero initialization, and auto casted
 *	to the specified type. This only gives an handy syntax for all allocations.
 *	If zero initialization if not needed or unwanted, just use the original
 *	malloc() function.
 */
#define mem_new(type, cnt) \
	(type *) calloc(sizeof(type), (cnt))

/** Free memory and reinit pointee.
 *
 *	Act like the original free(), but with pointer zero'ed. In fact, mem_free()
 *	do:
 *	@code
 *	free(*mem);
 *	*mem = NULL;
 *	@endcode
 *	if @a mem is a valid pointer, and always returns NULL (useful to free
 *	memory and return in the same instruction).
 */
void mem_free(void **pmem);

/** Reallocate memory, free it if error.
 *
 *	The mem_realloc() function do the same job that realloc(), but if it fails,
 *	it automatically frees the buffer. Of course, the freeing is done only if
 *	@a pmem and @a *pmem are not null.
 *
 *	Another trick is that if count equals zero, mem_realloc() acts like
 *	@ref mem_free(), providing a generic way to allocate and deallocate memory
 *	according to the only count parameter.
 */
void *mem_realloc(void **pmem, size_t count);

/** C++ style heap memory reallocation.
 *
 *	mem_renew() macro is a shortcut to @ref mem_realloc() with a syntax similar
 *	to @ref mem_new().
 */
#define mem_renew(mem, type, cnt) \
	(type *) mem_realloc((void **)(&(mem)), sizeof(type) * (cnt))

/** Copy newly allocated memory.
 *
 *	Acts like strdup(), but with memory. In fact, this's a shortcut to malloc()
 *	and memcpy() combined.
 */
void *mem_dup(void *mem, size_t size);

/** C++ style heap memory duplicate.
 *
 *	mem_dupt() macro is a shortcut to @ref mem_dup() with a syntax similar to
 *	@ref mem_new(), making cast implicit by the use of the source type.
 */
#define mem_dupt(type, mem, cnt) \
	(type *) mem_dup((void *) (mem), sizeof(type) * (cnt))

SCELIB_END_CDECL

#endif /* __SCELIB_MEMORY_H */
/* vi:set ts=4 sw=4: */
