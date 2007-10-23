/*	scelib - Simple C Extension Library
 *  Copyright (C) 2005-2007 Richard 'riri' GILL <richard@houbathecat.info>
 *
 *  defs.h - global definitions header.
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

#ifndef __SCELIB_DEFS_H
#define __SCELIB_DEFS_H

#ifdef __cplusplus
#define SCELIB_BEGIN_CDECL extern "C" {
#define SCELIB_END_CDECL }
#else
#define SCELIB_BEGIN_CDECL
#define SCELIB_END_CDECL
#endif

/* ------------------------------------------------------------------------- */
/* Sets errno to the specified error number, and returns given return code.  */
#define RETERROR(error, ret) \
	(errno = (error), (ret))

/* ------------------------------------------------------------------------- */
/* Saves the errno before any manipulation.                                  */
#define SAFEERRNO(code) \
	{ int err = errno; code; errno = err; }

#endif /* __SCELIB_DEFS_H */
/* vi:set ts=4 sw=4: */
