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

#include <stdlib.h>

_begin_cdecl



/* ========================================================================= */
/* memory functions                                                          */

/* ------------------------------------------------------------------------- */
/* mem_init()                                                                */
/* initialize ptr with val only if ptr is a valid pointer */

#define mem_init(ptr, val) \
	if (ptr) { *(ptr) = (val); }

/* ------------------------------------------------------------------------- */
/* mem_new()                                                                 */
/* shortcut for memory allocation (with me zeroed) */

#define mem_new(type, cnt) \
	(type *) calloc(sizeof(type), (cnt))

/* ------------------------------------------------------------------------- */
/* mem_free()                                                                */
/* free memory like free() do, but also reinit pointer to NULL.              */

void mem_free(void **pmem);

/* ------------------------------------------------------------------------- */
/* mem_realloc()                                                             */
/* do the same job that the standard realloc() function, but if it fails,    */
/* automatically free the input buffer pointed by pmem, and initialize the   */
/* pointer to null. Returns the equivalent of *pmem.                         */
/* If count == 0, mem_realloc() acts like mem_free().                        */

void *mem_realloc(void **pmem, size_t count);

/* ------------------------------------------------------------------------- */
/* mem_renew()                                                               */
/* shortcut to mem_realloc() with mem_new() semantics (type spec)            */

#define mem_renew(mem, type, cnt) \
	(type *) mem_realloc((void **)(&(mem)), sizeof(type) * (cnt))



/* ========================================================================= */
/* command line utilities                                                    */

/* ------------------------------------------------------------------------- */
/* style flags                                                               */

/* options naming styles */
#define CMDNAME_SHORT		0x01	/* -o */
#define CMDNAME_LONG		0x02	/* -option */
#define CMDNAME_MASK		(CMDNAME_SHORT|CMDNAME_LONG)

/* options / arguments separation styles */
#define CMDSEP_SPACE		0x04	/* option argument */
#define CMDSEP_EQUAL		0x08	/* option=argument */
#define CMDSEP_MASK			(CMDSEP_SPACE|CMDSEP_EQUAL)

/* options prefix styles */
#define CMDPREFIX_EMPTY		0x10	/* no prefix */
#define CMDPREFIX_SHORT		0x20	/* - */
#define CMDPREFIX_LONG		0x40	/* -- */
#define CMDPREFIX_GNU		(CMDPREFIX_SHORT|CMDPREFIX_LONG)
#define CMDPREFIX_DOS		0x80	/* / */
#define CMDPREFIX_MASK		(CMDPREFIX_EMPTY|CMDPREFIX_SHORT|CMDPREFIX_LONG|CMDPREFIX_DOS)

/* common used styles */
#define CMDSTYLE_GNUSHORT	(CMDNAME_SHORT|CMDSEP_SPACE|CMDPREFIX_SHORT)
#define CMDSTYLE_GNULONG	(CMDNAME_LONG|CMDSEP_EQUAL|CMDPREFIX_LONG)
#define CMDSTYLE_DOS		(CMDNAME_MASK|CMDSEP_SPACE|CMDPREFIX_DOS)
#define CMDSTYLE_JAVA		(CMDSTYLE_GNUSHORT|CMDNAME_LONG)

/* ------------------------------------------------------------------------- */
/* arguments requirement flags                                               */

#define CMDARG_NONE			0x01	/* no argument expected */
#define CMDARG_OPT			0x02	/* optional argument accepted */
#define CMDARG_REQ			0x04	/* required argument expected */

/* ------------------------------------------------------------------------- */
/* type cmdline_t                                                            */
/* Hidden structure to manage the command line options.                      */

typedef struct cmdline_ *cmdline_t;

/* ------------------------------------------------------------------------- */
/* type cmdparsed_t                                                          */
/* Structure filled by the cmd_find(), cmd_getnext() and cmd_loop()          */
/* function for arguments retrieval.                                         */

typedef struct cmdparsed_ {
	char *name;
	char *arg;
} cmdparsed_t;

/* ------------------------------------------------------------------------- */
/* cmd_create()                                                              */
/* Create a new empty command line parser object.                            */
/* size is the initial size of options array (pre-allocation), and grow is   */
/* the array growing size in case of reallocation. Passing 0 implies default */
/* values (0 and 1 respectively).                                            */
cmdline_t cmd_create(int size, int grow);

/* ------------------------------------------------------------------------- */
/* cmd_destroy()                                                             */
/* Free all memory allocated for the cmdline_t object (do not use saved      */
/* string pointer after the freed !).                                        */
void cmd_destroy(cmdline_t cmd);

/* ------------------------------------------------------------------------- */
/* cmd_option_add()                                                          */
/* Add a option to the cmdline_t object. Give the option name (without       */
/* prefix), its style (or-ed flags, in which the prefix style is specified)  */
/* and the argument requirement for this option.                             */
/* The option id (in fact its position in the options array) is returned if  */
/* all is ok, -1 is returned otherwise (use errno to get the error).         */
int cmd_option_add(cmdline_t cmd, char *name, int style, int argreq);

/* ------------------------------------------------------------------------- */
/* cmd_option_add_alt()                                                      */
/* Add an alternate name and style for the option determined by its id       */
/* (returned by a previous call to cmd_option_add()). Returns 0 if ok, -1    */
/* otherwise (use errno to get the error).                                   */
int cmd_option_add_alt(cmdline_t cmd, int idx, char *altname, int style);

/* ------------------------------------------------------------------------- */
/* cmd_dump()                                                                */
/* Do an stdout output of defined options, useful for debugging.             */
void cmd_dump(cmdline_t cmd);

/* ------------------------------------------------------------------------- */
/* cmd_parse()                                                               */
/* Do the actual command line parsing. Defined options are checked, and non  */
/* defined one are stored in a 'trash' which can be retrieved after. Returns */
/* 0 if ok, -1 if an internal error occurred, or 1 if the command line did   */
/* not respect the defined options (sytles).                                 */
int cmd_parse(cmdline_t cmd, int argc, char **argv);

/* ------------------------------------------------------------------------- */
/* cmd_find()                                                                */
/* First kind of usage of the command line utilities: checks is the option   */
/* determined by its id was passed to the command line and if so, retrieves  */
/* the option name and argument (if one, NULL otherwise) and returns 0, -1   */
/* if not (with errno set). Do not free the out string parameters, they're   */
/* the internal cmdline_t buffers.                                           */
int cmd_find(cmdline_t cmd, int idx, cmdparsed_t *parsed);

/* ------------------------------------------------------------------------- */
/* cmd_getnext()                                                             */
/* Second kind of usage of the command line utilities: loops on each option  */
/* of the passed command line, checking if the option was previously defined */
/* in the cmdline_t object (returning 0) or not (returning 1). Note that if  */
/* the option is not recognized, options styles are not checked (like the    */
/* prefix) and then the arg field of the cmdparsed_t structure will always   */
/* be NULL, because the argument presence can't be checked. If an error      */
/* occurs, -1 is returned (and errno is set accordingly).                    */
/* Do not free the out string parameters, they're pointers to argv.          */
/* TODO: add a default style check for non defined options                   */
int cmd_getnext(cmdline_t cmd, cmdparsed_t *parsed);

/* ------------------------------------------------------------------------- */
/* cmd_loop()                                                                */
/* Third kind of usage of the command line utilities: loops on all defined   */
/* options, retrieving the actual passed name is so (and returning 0) and    */
/* argument is present, or returning 1 is this option wasn't passed. Returns */
/* -1 if any error (use errno to get the error).                             */
/* Do not free the out string parameters, they're the internal cmdline_t     */
/* buffers.                                                                  */
int cmd_loop(cmdline_t cmd, cmdparsed_t *parsed);

/* ------------------------------------------------------------------------- */
/* cmd_reset()                                                               */
/* The cmdline_t object uses internal counters to track its state between    */
/* calls of cmd_parse(), cmd_find() and cmd_loop(). This function reset      */
/* theses counters (returning 0, or -1 if any error, and use errno to get    */
/* the error).                                                               */
int cmd_reset(cmdline_t cmd);







/* ######################################################################### */
/* old code                                                                  */
/* ######################################################################### */
#if 0

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
/* cmdline_parse()                                                           */
/* Parse the supplied command line arguments and alter the options array as  */
/* needed.                                                                   */

int cmdline_parse(int argc, char **argv, cmdopt_t *opttab, int optcount);

#endif

/* ========================================================================= */
/* header's footer                                                           */

_end_cdecl

#endif	/* _SCELIB_H */
/* vi:set ts=4 sw=4: */
