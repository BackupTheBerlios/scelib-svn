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
 *	@brief Platform detection and checks.
 *
 *	This header isn't mandatory to use scelib, because most of the header
 *	files are platform independant. But it maybe useful to know the platform,
 *	the operating system, the architecture bitlength (32 or 64bits) to write
 *	platform dependant code not included with scelib.
 *
 *	For this, this file defines all components it supports, and provide both
 *	simple defines and checking macros to help you.
 */
#ifndef __SCELIB_PLATFORM_H
#define __SCELIB_PLATFORM_H

/* ------------------------------------------------------------------------- */
/* Global platform defines                                                   */

#define PLATFORM_WINDOWS		0x10	/**< Code for the Windows family */
#define PLATFORM_UNIX			0x20	/**< Code for the Unix family */

#define PLATFORM_UNIX_AIX		0x21	/**< Code for the AIX *nix */
#define PLATFORM_UNIX_HPUX		0x22	/**< Code for the HP-UX *nix */
#define PLATFORM_UNIX_TRU64		0x23	/**< Code for the OSF/Tru64/Digital *nix */
#define PLATFORM_UNIX_LINUX		0x24	/**< Code for the Linux (GNU?) *nix */
#define PLATFORM_UNIX_SUNOS		0x25	/**< Code for the SunOS *nix */

#define PLATFORM_ARCH_32		0x32	/**< Code for 32bits architecture */
#define PLATFORM_ARCH_64		0x64	/**< Code for 64bits architecture */

/* ------------------------------------------------------------------------- */
/* Platform checking macros                                                  */

/** Evaluate a test on the platform (OS family).
 *
 *	This macro evaluates to a true/false condition, depending on the platform
 *	being compiled. For each supported platform (for now @ref PLATFORM_WINDOWS
 *	"WINDOWS" and @ref PLATFORM_UNIX "UNIX" are accepted as argument), it
 *	indicates if the current one equals the stressed one.
 *
 *	@code
 *	#if 0
 *	#elif PLATFORM_IS(WINDOWS)
 *	//do windows specific stuff ...
 *	#elif PLATFORM_IS(UNIX)
 *	//do unix specific stuff
 *	#else
 *	#error go hell!
 *	#endif
 *	@endcode
 */
#define PLATFORM_IS(plat)		(PLATFORM && PLATFORM_##plat && \
								(PLATFORM == PLATFORM_##plat))

/** Evaluate a test on the Unix OS.
 *
 *	This macro evaluates to a true/false condition, depending on the *nix
 *	operating system supporting the current compilation. For each supported
 *	*nix, it indicates if the current one equals the stressed one.
 *
 *	For now, the following *nix codes are valid:
 *	@ref PLATFORM_UNIX_AIX "AIX",
 *	@ref PLATFORM_UNIX_HPUX "HPUX",
 *	@ref PLATFORM_UNIX_TRU64 "TRU64",
 *	@ref PLATFORM_UNIX_LINUX "LINUX",
 *	@ref PLATFORM_UNIX_SUNOS "SUNOS"
 *
 *	@code
 *	#if PLATFORM_UNIX_IS(LINUX)
 *	#include <linux/version.h>
 *	#endif
 *	@endcode
 */
#define PLATFORM_UNIX_IS(os)	(PLATFORM_UNIXOS && PLATFORM_UNIX_##os && \
								(PLATFORM_UNIXOS == PLATFORM_UNIX_##os))

/** Evaluate a test on the bitness of the architecture.
 *
 *	It's sometime usefull to know if we operate on a @ref PLATFORM_ARCH_32
 *	"32bits" system or a @ref PLATFORM_ARCH_64 "64bits" one. This macro
 *	evaluates to a true/false condition which gives you this information.
 *
 *	@code
 *	#if PLATFORM_ARCH_IS(64)
 *	// use 64bits stuff
 *	#else
 *	// use 32bits stuff
 *	#endif
 *	@endcode
 */
#define PLATFORM_ARCH_IS(bits)	(PLATFORM_ARCH && PLATFORM_ARCH_##bits && \
								(PLATFORM_ARCH == PLATFORM_ARCH_##bits))

#if !defined(DOXYGEN_PARSING)

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
#define PLATFORM_DIRSEP			"\\"
#define PLATFORM_DIRSEPC		'\\'
#define PLATFORM_PATHSEP		";"
#define PLATFORM_PATHSEPC		';'
#define PLATFORM_EOL			"\x0D\x0A"

#else	/* !(WIN32 || WIN64) */

/* ------------------------------------------------------------------------- */
/* Unix platform                                                             */

#define PLATFORM				PLATFORM_UNIX

#define PLATFORM_LIBPREFIX		"lib"
#define PLATFORM_BINEXT			""
#define PLATFORM_DIRSEP			"/"
#define PLATFORM_DIRSEPC		'/'
#define PLATFORM_PATHSEP		":"
#define PLATFORM_PATHSEPC		':'
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

/* end of platforms */
#endif	/* WIN32 || WIN64 */

#else	/* DOXYGEN_PARSING */

/** Actual bitness architecture (platform dependent).
 *
 *	Can be one of @ref PLATFORM_ARCH_32 and @ref PLATFORM_ARCH_64 defines.
 */
#define PLATFORM_ARCH

/** Actual platform family (platform dependent).
 *
 *	Can be one of @ref PLATFORM_WINDOWS and @ref PLATFORM_UNIX.
 */
#define PLATFORM

/** Actual Unix operating system (OS dependent).
 *
 *	If the platform in of the *nix family (see @ref PLATFORM), can be one of
 *	@ref PLATFORM_UNIX_AIX, @ref PLATFORM_UNIX_HPUX, @ref PLATFORM_UNIX_TRU64,
 *	@ref PLATFORM_UNIX_LINUX and @ref PLATFORM_UNIX_SUNOS.
 */
#define PLATFORM_UNIXOS

/** Actual prefix used before library names (platform dependent).
 *
 *	Usually, this's 'lib' on *nix platform, and empty on Windows.
 */
#define PLATFORM_LIBPREFIX

/** Actual shared library file extension (platform dependent).
 *
 *	The shared library filenames have different extensions depending on the
 *	underlying operating system. Windows uses '.dll', but on *nix, even if
 *	the '.so' extension is widely used, several OS use different standard
 *	extensions ('.sl' on HP-UX, '.a' on Aix, and so on).
 */
#define PLATFORM_LIBEXT

/** Actual binary executable file extension (platform dependent).
 *
 *	Usually, this's '.exe' on Windows and empty on *nix.
 */
#define PLATFORM_BINEXT

/** Actual directory separator character (platform dependent).
 *
 *	Always '/' on *nix and '\' on Windows.
 */
#define PLATFORM_DIRSEPC

/** Actual directory separator string (platform dependent).
 *
 *	Same as @ref PLATFORM_DIRSEPC but in the C-string form.
 */
#define PLATFORM_DIRSEP

/** Actual path separator character (platform dependent).
 *
 *	Always ':' on *nix and ';' on Windows.
 */
#define PLATFORM_PATHSEPC

/** Actual path separator string (platform dependent).
 *
 *	Same as @ref PLATFORM_PATHSEPC but in the C-string form.
 */
#define PLATFORM_PATHSEP

/** Actual end-of-line string (platform dependent).
 *
 *	The native suite of characters used to make a textual end-of-line.
 *	This's always "\x0D\x0A" on Windows and "\x0A" on *nix.
 */
#define PLATFORM_EOL

#endif	/* !DOXYGEN_PARSING */

#endif /* __SCELIB_PLATFORM_H */
/* vi:set ts=4 sw=4: */
