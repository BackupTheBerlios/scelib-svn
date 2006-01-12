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
/* command line functions                                                    */

/*
 * Options can begin with a short specifier '-', a long one '--', or either
 * of these two styles.
 */

#define CMDFLAG_SHORT		1
#define CMDFLAG_LONG		2
#define CMDFLAG_BOTH		(CMDFLAG_SHORT|CMDFLAG_LONG)

/*
 * Argument attached to an option can be required, optional, or not permited.
 */

#define CMDARG_NONE			1
#define CMDARG_OPT			2
#define CMDARG_REQ			3

/*
 * When an argument is specified, the user has to attach it to the option with
 * an equal sign, without one, or either of these two options.
 */

#define CMDFLAG_EQUAL		1
#define CMDFLAG_NOEQUAL		2
#define CMDFLAG_EITHER		(CMDFLAG_EQUAL|CMDFLAG_NOEQUAL)

/*
 * When a command line option is not specified, the optidx field is valued
 * with CMDERR_NOSPEC, telling the parser didn't found it.
 */

#define CMDERR_NOSPEC		-1

/*
 * When the parser find an option specification which doesn't match the
 * option's style (argument optionality and equal sign optionality), it
 * affects the error field, putting CMDERR_OK when all watch as wanted.
 */

#define CMDERR_OK			0
#define CMDERR_EQUAL		1
#define CMDERR_ARG			2

/*
 * An option name has a limited number of characters.
 */

#define CMDOPT_NAMELEN	31

/*
 * The options the parser has to check are specified in an array of cmdopt_t
 * structures, one element identifying the wanted option. the developper fills
 * the first set of fields to give the option 'personality'. When the parser
 * returns, it has filled the second set of fields with appropriate values.
 * When an option not expected is encountered, it can be put in a 'trash', a
 * special option a developer has put in the array, with all the fields at
 * 0 or NULL (as needed by the field type). In this case, only the 'arg' field
 * will be filled, with a conglomerate of all unrecognized options and their
 * argument is one.
 */

typedef struct cmdopt {

	/* first set, option personality */
	int style;		/* short, long or both */
	int argreq;		/* none, optional or required */
	int eqstyle;	/* equel sign, no sign, or either */
	char *opt;		/* option name (should respect CMDOPT_NAMELEN) */

	/* second set, parser filled fields */
	int optidx;		/* option index (CMDERR_NOSPEC if not specified) */
	int error;		/* see CMDERR_* defines /
	/*
	 * Provided even if the argument was not expected; if argument is not
	 * specified, this field is valued with NULL.
	 */
	char *arg;

} cmdopt_t;

/* ------------------------------------------------------------------------- */
/* sceparse_cmdline()                                                        */
/* Parse the supplied command line arguments and alter the options array as  */
/* needed.                                                                   */

int sceparse_cmdline(int argc, char **argv, cmdopt_t *opttab, int optcount);


/* ========================================================================= */
/* header's footer                                                           */

_end_cdecl

#endif	/* _SCELIB_H */
/* vi:set ts=4 sw=4: */
