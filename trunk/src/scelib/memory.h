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

#ifndef __SCELIB_MEMORY_H
#define __SCELIB_MEMORY_H

#include "defs.h"
#include "platform.h"
#include <stdlib.h>

SCELIB_BEGIN_CDECL

#define mem_init(ptr, val) \
	if (ptr) { *(ptr) = (val); }

#define mem_new(type, cnt) \
	(type *) calloc(sizeof(type), (cnt))

void mem_free(void **pmem);

void *mem_realloc(void **pmem, size_t count);

#define mem_renew(mem, type, cnt) \
	(type *) mem_realloc((void **)(&(mem)), sizeof(type) * (cnt))

void *mem_dup(void *mem, size_t size);

#define mem_dupt(type, mem, cnt) \
	(type *) mem_dup((void *) (mem), sizeof(type) * (cnt))

#if PLATFORM_IS(WINDOWS)
#define mem_zero(mem, size) \
	ZeroMemory(mem, size), (mem)
#else
#define mem_zero(mem, size) \
	memset(mem, 0, size)
#endif

#define mem_zerot(type, mem, cnt) \
	(type *) mem_zero(mem, sizeof(type) * (cnt))

SCELIB_END_CDECL

#endif /* __SCELIB_MEMORY_H */
/* vi:set ts=4 sw=4: */
