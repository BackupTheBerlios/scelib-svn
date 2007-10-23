/*	scelib - Simple C Extension Library
 *  Copyright (C) 2005-2007 Richard 'riri' GILL <richard@houbathecat.info>
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
 *	@brief Global definitions header.
 *
 *	Even if this header isn't quit interesting, it's important for other
 *	scelib headers, because it defines common things used everywhere,
 *	plus some handy tools.
 */

#ifndef __SCELIB_DEFS_H
#define __SCELIB_DEFS_H

/** @def SCELIB_BEGIN_CDECL
 *	@brief Start an 'extern "C"' declarations block
 */
/** @def SCELIB_END_CDECL
 *	@brief End a C declarations block
 */
#if defined(__cplusplus)
#define SCELIB_BEGIN_CDECL extern "C" {
#define SCELIB_END_CDECL }
#else
#define SCELIB_BEGIN_CDECL
#define SCELIB_END_CDECL
#endif

/** Set errno to the specified error number, and returns the code.
 *
 *	This simple macro is handy when you have several return statements in a
 *	body and if you use the errno machanism to report errors, because it
 *	provide a short notation for setting the error, and returning the code
 *	(which is often -1).
 */
#define RETERROR(error, ret) \
	(errno = (error), (ret))

/** Saves errno before any manipulation.
 *
 *	This macro can be used together with the RETERROR() one, when you need
 *	to perform some actions without being sure errno will stay constant (for
 *	example if a malloc() fails and you need to do some cleanup before
 *	exiting).
 */
#define SAFEERRNO(code) \
	{ int err = errno; code; errno = err; }

#endif /* __SCELIB_DEFS_H */
/* vi:set ts=4 sw=4: */
