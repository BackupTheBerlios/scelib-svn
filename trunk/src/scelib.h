/*
 *  scelib - Simpliest C Extension Library
 *  Copyright (C) 2005-2006 Richard 'riri' GILL <richard@houbathecat.info>
 *
 *  scelib.h - header file of the library.
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
#include <stdio.h>
#include <errno.h>

_begin_cdecl



/* ========================================================================= */
/* general utilities                                                         */

/* ------------------------------------------------------------------------- */
/* reterror()                                                                */
/* Sets errno to the specified error number, and returns given return code.  */

#define reterror(error, ret) \
	(errno = (error), (ret))



/* ========================================================================= */
/* memory functions                                                          */

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

#ifdef PLAT_WINDOWS
#define mem_zero(mem, size) \
	ZeroMemory(mem, size), (mem)
#else
#define mem_zero(mem, size) \
	memset(mem, 0, size)
#endif

#define mem_zerot(type, mem, cnt) \
	(type *) mem_zero(mem, sizeof(type) * (cnt))



/* ========================================================================= */
/* command line utilities                                                    */

typedef enum argsep_ {

	ARGSEP_NONE = 0,	/* special value for options without argument  */
	ARGSEP_SPACE,		/* separate argument from option with a space */
	ARGSEP_EQUAL		/* separate argument from option with a '=' sign */

} e_argsep;

typedef enum argreq_ {

	ARG_NONE = 0,		/* no argument expected */
	ARG_OPTIONAL,		/* optional argument accepted */
	ARG_REQUIRED		/* argument is required along the option */

} e_argreq;

typedef struct cmdline_ *cmdline_t;

typedef struct cmdparsed_ cmdparsed_t;
struct cmdparsed_ {

	/* option values */
	char *name;			/* actual option name */
	char *arg;			/* argument if given, NULL otherwise */
	int argpos;			/* argc value of this element */

	/* defined option related */
	int error;			/* CMDERR_* error codes if something went wrong */
	int optid;			/* option id if matched, -1 otherwise */
	int nameid;			/* name id if matched, -1 otherwise */

};

#define CMDERR_REQ	1	/* argument requirement error */
#define CMDERR_SEP	2	/* argument separation style error */

typedef int (*cmdparse_cb)(cmdline_t cmd, cmdparsed_t *parsed, void *userdata);

/* ------------------------------------------------------------------------- */
cmdline_t cmd_create(cmdparse_cb cb);
void cmd_destroy(cmdline_t cmd);
int cmd_addopt(cmdline_t cmd, char *name, char *descr, cmdparse_cb cb);
int cmd_addopt_arg(cmdline_t cmd, char *name, e_argreq argreq, e_argsep argsep, char *descr, char *argdescr, cmdparse_cb cb);
int cmd_addopt_name(cmdline_t cmd, int optid, char *altname, e_argsep altsep);
int cmd_getopt(cmdline_t cmd, int optid, int nameid, char **name, e_argreq *argreq, e_argsep *argsep, char **descr, char **argdescr);
void cmd_print(FILE *fd, cmdline_t cmd, char *head, char *tail);
int cmd_parse(cmdline_t cmd, int argc, char **argv, void *userdata);



/* ========================================================================= */
/* string functions                                                          */



/* ========================================================================= */
/* header's footer                                                           */

_end_cdecl

#endif	/* _SCELIB_H */
/* vi:set ts=4 sw=4: */
