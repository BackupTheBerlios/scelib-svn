/*	scelib - Simple C Extension Library
 *  Copyright (C) 2005-2007 Richard 'riri' GILL <richard@houbathecat.info>
 *
 *  platform.h - platform detection and checks.
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

#ifndef __SCELIB_PLATFORM_H
#define __SCELIB_PLATFORM_H

/* ------------------------------------------------------------------------- */
/* Global platform defines                                                   */

#define PLATFORM_WINDOWS		0x10
#define PLATFORM_UNIX			0x20

#define PLATFORM_UNIX_AIX		0x21
#define PLATFORM_UNIX_HPUX		0x22
#define PLATFORM_UNIX_TRU64		0x23
#define PLATFORM_UNIX_LINUX		0x24
#define PLATFORM_UNIX_SUNOS		0x25

#define PLATFORM_ARCH_32		0x32
#define PLATFORM_ARCH_64		0x64

/* ------------------------------------------------------------------------- */
/* Platform checking macros                                                  */

#define PLATFORM_IS(plat)		(PLATFORM && PLATFORM_##plat && \
								(PLATFORM == PLATFORM_##plat))
#define PLATFORM_UNIX_IS(os)	(PLATFORM_UNIXOS && PLATFORM_UNIX_##os && \
								(PLATFORM_UNIXOS == PLATFORM_UNIX_##os))
#define PLATFORM_ARCH_IS(bits)	(PLATFORM_ARCH && PLATFORM_ARCH_##bits && \
								(PLATFORM_ARCH == PLATFORM_ARCH_##bits))

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)

/* ------------------------------------------------------------------------- */
/* Windows platform                                                          */

#if defined(WIN64) || defined(_WIN64)
#error The 64bits Windows platform is not supported yet!
#define PLATFORM_ARCH			PLATFORM_ARCH_64
#else
#define PLATFORM_ARCH			PLATFORM_ARCH_32
#endif	/* ARCH */
#define PLATFORM				PLATFORM_WINDOWS

#define PLATFORM_LIBPREFIX		""
#define PLATFORM_LIBEXT			".dll"
#define PLATFORM_BINEXT			".exe"
#define PLATFORM_PATHSEP		"\\"
#define PLATFORM_PATHSEPC		'\\'
#define PLATFORM_EOL			"\x0D\x0A"

#define WIN32_LEAN_AND_MEAN		/* remove unusual definitions */
#define STRICT					/* strict type checking */
#define _WIN32_WINNT	0x0500	/* Windows 2000 minimum */
#include <windows.h>

#else	/* !(WIN32 || WIN64) */

/* ------------------------------------------------------------------------- */
/* Unix platform                                                             */

#define PLATFORM				PLATFORM_UNIX

#define PLATFORM_LIBPREFIX		"lib"
#define PLATFORM_BINEXT			""
#define PLATFORM_PATHSEP		"/"
#define PLATFORM_PATHSEPC		'/'
#define PLATFORM_EOL			"\x0A"

#if defined(_AIX) || defined(__TOS_AIX__)
#define PLATFORM_UNIXOS			PLATFORM_UNIX_AIX
#if defined(__64BIT__)
#define PLATFORM_ARCH			PLATFORM_ARCH_64
#endif
#define PLATFORM_LIBEXT			".a"
#elif defined(hpux) || defined(_hpux) || defined(__hpux)
#define PLATFORM_UNIXOS			PLATFORM_UNIX_HPUX
#if defined(__LP64__)
#define PLATFORM_ARCH			PLATFORM_ARCH_64
#endif
#define PLATFORM_LIBEXT			".sl"
#elif defined(__digital__) || defined(__osf__)
#define PLATFORM_UNIXOS			PLATFORM_UNIX_TRU64
#define PLATFORM_ARCH			PLATFORM_ARCH_64
#define PLATFORM_LIBEXT			".so"
#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__TOS_LINUX__) || defined(__CYGWIN__)
#define PLATFORM_UNIXOS			PLATFORM_UNIX_LINUX
/* TODO: detect if the processor is a 64bits one (can be), and error in this case */
#define PLATFORM_LIBEXT			".so"
#else
#error Unrecognized Platform (new Unix one ?)
#endif

/* defaults to 32bits */
#if !defined(PLATFORM_ARCH)
#define PLATFORM_ARCH			PLATFORM_ARCH_32
#endif

#include <pthread.h>
#include <sched.h>

/* end of platforms */
#endif	/* WIN32 || WIN64 */

#endif /* __SCELIB_PLATFORM_H */
/* vi:set ts=4 sw=4: */
