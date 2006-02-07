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
#include <stdio.h>	/* for FILE * */

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

/* if no argument for the option, use this as the 'name style' */
#define CMDSTYLE_NONE		0
/* the argument, when present, will be separated from the option by a space */
#define CMDSTYLE_SPACE		1
/* the argument will be separated from the option by an equal sign */
#define CMDSTYLE_EQUAL		2

/* ------------------------------------------------------------------------- */
/* arguments requirement flags                                               */
/* NOTE: the CMDARG_OPT detection seems to be hard to code */

#define CMDARG_NONE			0	/* no argument expected */
#define CMDARG_OPT			1	/* optional argument accepted */
#define CMDARG_REQ			2	/* required argument expected */

/* ------------------------------------------------------------------------- */
/* option repetition special behaviors                                       */

#define CMDREPET_UNIQUE		1
#define CMDREPET_ANY		-1

/* ------------------------------------------------------------------------- */
/* type cmdline_t                                                            */
/* Hidden structure to manage the command line options.                      */

typedef struct cmdline_ *cmdline_t;

/* ------------------------------------------------------------------------- */
/* type cmdparsed_t                                                          */
/* Structure filled by the cmd_find(), cmd_getnext() and cmd_loop()          */
/* function for arguments retrieval.                                         */
#if 0
typedef struct cmdparsed_ {

	char *name;	/* actual option name */
	char *arg;	/* argument if revelant, NULL otherwise */
	int style;	/* option naming style */

	int optid;	/* option index if found */
	int error;	/* error code if found but with error */

} cmdparsed_t;
#endif

/* ------------------------------------------------------------------------- */
/* cmd_create()                                                              */
/* Creates a new empty command line parser object.                           */
/* Returns a new command line tool object.                                   */

cmdline_t cmd_create(int argc, char **argv);

/* ------------------------------------------------------------------------- */
/* cmd_create_ex()                                                           */
/* Creates a new empty command line parser object, specifying the growing    */
/* of the internal options array (for performance issues). The default grow  */
/* size is 10.                                                               */
/* Returns a new command line tool object.                                   */

cmdline_t cmd_create_ex(int argc, char **argv, int grow);

/* ------------------------------------------------------------------------- */
/* cmd_destroy()                                                             */
/* Free all memory allocated for the cmdline_t object (do not use saved      */
/* string pointer after the freed !).                                        */

void cmd_destroy(cmdline_t cmd);

/* ------------------------------------------------------------------------- */
/* cmd_add_opt()                                                             */
/* Adds to the command line tool an option which doesn't take an argument.   */
/* Just specify the option name, and eventually a description if command     */
/* line help is to be used.                                                  */
/* Returns the identifier (in fact, the index in the options array) for the  */
/* option.                                                                   */

int cmd_addopt(cmdline_t cmd, char *name, char *descr);

/* ------------------------------------------------------------------------- */
/* cmd_add_opt_arg()                                                         */
/* Adds to the command line tool an option which *may* take an argument.     */
/* Specify the option name, the argument separation style (CMDSTYLE_*), the  */
/* argument requirement flag (CMDARG_*), and eventually a description for    */
/* option and a text for the argument if command line help is to be used. If */
/* the argdescr is NULL, the term 'value' will be used.                      */
/* Returns the identifier (in fact, the index in the options array) for the  */
/* option.                                                                   */

int cmd_addopt_arg(cmdline_t cmd, char *name, int style, int argreq,
	char *descr, char *argdescr);

/* ------------------------------------------------------------------------- */
/* cmd_addopt_name()                                                         */
/* Adds an alternate name for the option specified by its identifier. Just   */
/* give its name and the argument separation style (CMDSTYLE_*) - use the    */
/* CMDSTYLE_NONE value if no argument expected.                              */

int cmd_addopt_name(cmdline_t cmd, int idx, char *altname, int altstyle);

/* ------------------------------------------------------------------------- */
/* cmd_print()                                                               */
/* Print a quick command line help with information passed during options    */
/* creation.                                                                 */
/* head and tail can be NULL. For each option description, try to add a      */
/* tabulation character '\t' after each new line '\n' because the option     */
/* description starts with a tabulation.                                     */

void cmd_print(FILE *fd, cmdline_t cmd, char *head, char *tail);

/* NOTE: seems I'll just code cmd_parse() to initialize parser, and
 * cmd_getnext() to loop on passed arguments
 */

/* ------------------------------------------------------------------------- */
/* cmd_parse()                                                               */
/* Do the actual command line parsing. Defined options are checked, and non  */
/* defined one are stored in a 'trash' which can be retrieved after. Returns */
/* 0 if ok, -1 if an internal error occurred, or 1 if the command line did   */
/* not respect the defined options (sytles).                                 */
/*int cmd_parse(cmdline_t cmd, int argc, char **argv);*/

/* ------------------------------------------------------------------------- */
/* cmd_find()                                                                */
/* First kind of usage of the command line utilities: checks is the option   */
/* determined by its id was passed to the command line and if so, retrieves  */
/* the option name and argument (if one, NULL otherwise) and returns 0, -1   */
/* if not (with errno set). Do not free the out string parameters, they're   */
/* the internal cmdline_t buffers.                                           */
/*int cmd_find(cmdline_t cmd, int idx, int argc, char **argv, cmdparsed_t *parsed);*/

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

/* -1 if error
 * 0 if not found
 * 1 if found
 */
/*int cmd_getnext(cmdline_t cmd, cmdparsed_t *parsed);*/

/* ------------------------------------------------------------------------- */
/* cmd_loop()                                                                */
/* Third kind of usage of the command line utilities: loops on all defined   */
/* options, retrieving the actual passed name is so (and returning 0) and    */
/* argument if present, or returning 1 is this option wasn't passed. Returns */
/* -1 if any error (use errno to get the error).                             */
/* Do not free the out string parameters, they're the internal cmdline_t     */
/* buffers.                                                                  */
/*int cmd_loop(cmdline_t cmd, cmdparsed_t *parsed);*/

/* ------------------------------------------------------------------------- */
/* cmd_reset()                                                               */
/* The cmdline_t object uses internal counters to track its state between    */
/* calls of cmd_parse(), cmd_find() and cmd_loop(). This function reset      */
/* theses counters (returning 0, or -1 if any error, and use errno to get    */
/* the error).                                                               */
/*int cmd_reset(cmdline_t cmd);*/





/* ========================================================================= */
/* header's footer                                                           */

_end_cdecl

#endif	/* _SCELIB_H */
/* vi:set ts=4 sw=4: */
