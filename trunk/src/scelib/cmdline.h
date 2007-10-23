/*	scelib - Simple C Extension Library
 *  Copyright (C) 2005-2007 Richard 'riri' GILL <richard@houbathecat.info>
 *
 *  cmdline.h - command line handling declarations.
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

#ifndef __SCELIB_CMDLINE_H
#define __SCELIB_CMDLINE_H

#include "defs.h"
#include "platform.h"

SCELIB_BEGIN_CDECL

#define CMDF_ENDMARK	0x0001	/* -- is end of options */

#define CMDF_DASHSHORT	0x0002	/* - for short option */
#define CMDF_DASHLONG	0x0004	/* - for long option */
#define CMDF_DDASHLONG	0x0008	/* -- for long option */

#define CMDF_SLASHSHORT	0x0010	/* / for short option */
#define CMDF_SLASHLONG	0x0020	/* / for long option */

#define CMDF_EQUALSHORT	0x0100	/* = for short option's argument */
#define CMDF_EQUALLONG	0x0200	/* = for long option's argument */
#define CMDF_SPACESHORT	0x0400	/* ' ' for short option's argument */
#define CMDF_SPACELONG	0x0800	/* ' ' for long option's argument */

#define CMDF_ONLYDEFS	0x1000	/* callback only defined options */

/* -s, -s arg, --long, --long=arg, -- */
#define CMDF_GNUSTYLE	CMDF_DASHSHORT|CMDF_DDASHLONG|CMDF_SPACESHORT|CMDF_EQUALLONG
/* /s, /s arg, /long, /long arg */
#define CMDF_WINSTYLE	CMDF_SLASHSHORT|CMDF_SLASHLONG|CMDF_SPACESHORT|CMDF_SPACELONG
/* -s, -s=arg, -long, -long=arg */
#define CMDF_JAVASTYLE	CMDF_DASHSHORT|CMDF_DASHLONG|CMDF_SPACESHORT|CMDF_SPACELONG

/* GNU or Windows */
#if PLATFORM_IS(WINDOWS)
#define CMDF_STDSTYLE	CMDF_WINSTYLE
#else
#define CMDF_STDSTYLE	CMDF_GNUSTYLE
#endif

typedef struct cmdline_type *cmdline_t;
typedef struct cmdparsed_type cmdparsed_t;
typedef int (*cmdline_cb_t)(cmdline_t, cmdparsed_t *, void *);

struct cmdparsed_type
{
	int opt_pos;	/* position on command line */
	char *opt_name;	/* option name, without prefix */
	char *opt_arg;	/* argument, if specified, NULL otherwise */

	int optid;		/* reference to option id, -1 if not a defined one */
};

cmdline_t cmdline_create(int flags, cmdline_cb_t callback);
int cmdline_destroy(cmdline_t cl);
int cmdline_parse(cmdline_t cl, int argc, char **argv, void* userdata);

int cmdline_addopt(cmdline_t cl, char sname, char *lname,
				   cmdline_cb_t callback);
int cmdline_addopt_if(cmdline_t cl, char sname, char *lname,
					  cmdline_cb_t callback, int pred);
int cmdline_getopt(cmdline_t cl, int optid, char *sname, char **lname);

SCELIB_END_CDECL

#endif /* __SCELIB_CMDLINE_H */
/* vi:set ts=4 sw=4: */
