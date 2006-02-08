/*
 *  scelib - Simpliest C Extension Library
 *  Copyright (C) 2005-2006 Richard 'riri' GILL <richard@houbathecat.info>
 *
 *  cmdline.c - cool command line parser.
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

/* TODO:
 * - add support for the empty option (""), resulting during parsing in
 *   next argument retrieve
 * - if NULL description, do not print the option. To print it without descr,
 *   pass an empty string ("")
 * - see to remove support on CMDSTYLE_NONE with an argument (option and
 *   argument stucked)
 */

#include "scelib.h"
#include <string.h>
#include <errno.h>

/* ========================================================================= */
/* Types                                                                     */

/* ------------------------------------------------------------------------- */
/* type cmdname_t                                                            */
/* Quick helper to store option names (first and alternates) with their size */
/* and the style for argument separation.                                    */

typedef struct cmdname_ cmdname_t;
struct cmdname_ {

	char *name;			/* option name */
	int len;			/* length of name */
	int style;			/* argument separator style */

	cmdname_t *next;	/* for linked list */

};

/* ------------------------------------------------------------------------- */
/* type cmdopt_t                                                             */
/* Defines one option: its name(s), the number of defined names, the size of */
/* the names array growing by CMDNAME_GROWSIZE (for performance issues), the */
/* argument requirement flag, the description and argument text for help.    */

typedef struct cmdopt_ cmdopt_t;
struct cmdopt_ {

	cmdname_t *head;	/* head of linked list of names */
	cmdname_t *tail;	/* tail of linked list of names */
	int namec;			/* number of names */

	int argreq;			/* argument requirement flag */
	char *descr;		/* option description (for help) */
	char *argdescr;		/* argument description (for help) */

	cmdopt_t *next;		/* for linked list */

};

/* ------------------------------------------------------------------------- */
/* type cmdline_t                                                            */
/* Overall structure to manage command line options.                         */
/* The opts/optc/osize triplet acts like the names/namec/nsize one in the    */
/* cmdopt_ structure. The grow size can be specified at tool creation, or    */
/* will have the CMDOPT_GROWSIZE default value.                              */
/* argc and argv are stored are tool creation to simplify parser functions.  */

struct cmdline_ {

	cmdopt_t *head;		/* head of linked list of options */
	cmdopt_t *tail;		/* tail of linked list of options */
	int optc;			/* number of options */

	int argc;			/* saved argc */
	char **argv;		/* saved argv */

};



/* ========================================================================= */
/* Static functions declaration                                              */

/* ------------------------------------------------------------------------- */
/* cmd_opt_new()                                                             */
/* Creates a new cmdopt_ structure.                                          */

static cmdopt_t *cmd_opt_new(char *name, int style, int argreq, char *descr,
	char *argdescr);

/* ------------------------------------------------------------------------- */
/* cmd_opt_newname()                                                         */
/* Adds an alternate name to the specified option.                           */

static int cmd_opt_newname(cmdopt_t *opt, char *altname, int altstyle);

/* ------------------------------------------------------------------------- */
/* cmd_opt_del()                                                             */
/* Frees the specified option, with all allocated memory during option life. */

static void cmd_optdel(cmdopt_t *opt);



/* ========================================================================= */
/* Public functions definitions                                              */

/* ------------------------------------------------------------------------- */
/* cmd_create()                                                              */

cmdline_t cmd_create(int argc, char **argv) {

	cmdline_t cmd;

	cmd = (cmdline_t) calloc(1, sizeof(struct cmdline_));
	if (cmd == NULL) {
		return NULL;
	}

	cmd->argc = argc;
	cmd->argv = argv;

	return cmd;

}

/* ------------------------------------------------------------------------- */
/* cmd_destroy()                                                             */

void cmd_destroy(cmdline_t cmd) {

	cmdopt_t *opt, *next;

	if (cmd == NULL) {
		return;
	}

	opt = cmd->head;
	while (opt) {
		next = opt->next;
		cmd_optdel(opt);
		opt = next;
	}
	free(cmd);

}

/* ------------------------------------------------------------------------- */
/* cmd_addopt()                                                              */

int cmd_addopt(cmdline_t cmd, char *name, char *descr) {
	return cmd_addopt_arg(cmd, name, CMDSTYLE_NONE, CMDARG_NONE, descr, NULL);
}

/* ------------------------------------------------------------------------- */
/* cmd_addopt_arg()                                                          */

int cmd_addopt_arg(cmdline_t cmd, char *name, int style, int argreq,
	char *descr, char *argdescr) {

	cmdopt_t *newopt;

	if (cmd == NULL) {
		errno = EINVAL;
		return -1;
	}

	newopt = cmd_opt_new(name, style, argreq, descr, argdescr);
	if (newopt == NULL) {
		return -1;
	}

	if (cmd->head == NULL) {
		cmd->head = newopt;
		cmd->tail = newopt;
	}
	else {
		cmd->tail->next = newopt;
		cmd->tail = newopt;
	}

	return cmd->optc ++;

}

/* ------------------------------------------------------------------------- */
/* cmd_addopt_name()                                                         */

int cmd_addopt_name(cmdline_t cmd, int idx, char *altname, int altstyle) {

	cmdopt_t *opt;

	if (cmd == NULL || idx >= cmd->optc || altname == NULL) {
		errno = EINVAL;
		return -1;
	}

	opt = cmd->head;
	while (idx--) {
		opt = opt->next;
	}
	return cmd_opt_newname(opt, altname, altstyle);

}



/* ------------------------------------------------------------------------- */
/* cmd_print()                                                               */

void cmd_print(FILE *fd, cmdline_t cmd, char *head, char *tail) {

	cmdopt_t *opt;
	cmdname_t *name;
	static char defargd[] = "value";

	if (head) {
		fprintf(fd, "%s\n", head);
	}

	if (cmd == NULL || cmd->optc == 0) {
		return;
	}

	opt = cmd->head;
	while (opt) {
		if (opt->descr == NULL) {
			opt = opt->next;
			continue;
		}

		name = opt->head;
		while (name) {
			if (opt->argreq == CMDARG_NONE) {
				fprintf(fd, "%s\n", name->name);
			}
			else {
				char *adesc = (opt->argdescr) ? opt->argdescr : defargd;
				char *format = NULL;

				switch (name->style) {
					case CMDSTYLE_NONE:
						format = "%s'%s'\n";
						break;
					case CMDSTYLE_SPACE:
						format = "%s '%s'\n";
						break;
					case CMDSTYLE_EQUAL:
						format = "%s='%s'\n";
				}
				fprintf(fd, format, name->name, adesc);
			}
			name = name->next;
		}
		fprintf(fd, "\t%s\n", opt->descr);
		opt = opt->next;
	}

	if (tail) {
		fprintf(fd, "%s\n", tail);
	}

}

/* ========================================================================= */
/* Static functions definition                                               */

/* ------------------------------------------------------------------------- */
/* cmd_opt_new()                                                             */

static cmdopt_t *cmd_opt_new(char *name, int style, int argreq, char *descr,
	char *argdescr) {

	cmdopt_t *opt;
	int saved;

	if (name == NULL) {
		errno = EINVAL;
		return NULL;
	}

	opt = (cmdopt_t *) calloc(1, sizeof(cmdopt_t));
	if (opt == NULL) {
		return NULL;
	}

	opt->head = (cmdname_t *) calloc(1, sizeof(cmdname_t));
	if (opt->head == NULL) {
		saved = errno;
		free(opt);
		errno = saved;
		return NULL;
	}

	opt->head->name = strdup(name);
	if (opt->head->name == NULL) {
		saved = errno;
		free(opt->head);
		free(opt);
		errno = saved;
		return NULL;
	}
	opt->head->len = (int) strlen(opt->head->name);
	opt->head->style = style;
	opt->tail = opt->head;
	opt->namec = 1;

	opt->argreq = argreq;
	if (descr != NULL) {
		opt->descr = strdup(descr);
	}
	if (argdescr != NULL) {
		opt->argdescr = strdup(argdescr);
	}

	return opt;

}

/* ------------------------------------------------------------------------- */
/* cmd_opt_newname()                                                         */

static int cmd_opt_newname(cmdopt_t *opt, char *altname, int altstyle) {

	cmdname_t *newname;

	if (opt == NULL || opt->head == NULL || opt->namec < 1 ||
		altname == NULL) {
		errno = EINVAL;
		return -1;
	}

	newname = (cmdname_t *) calloc(1, sizeof(cmdname_t));
	if (newname == NULL) {
		return -1;
	}

	newname->name = strdup(altname);
	if (newname->name == NULL) {
		return -1;
	}
	newname->len = (int) strlen(newname->name);
	newname->style = altstyle;
	opt->tail->next = newname;
	opt->tail = newname;

	return opt->namec ++;

}

/* ------------------------------------------------------------------------- */
/* cmd_opt_del()                                                             */

static void cmd_optdel(cmdopt_t *opt) {

	cmdname_t *name, *next;

	if (opt == NULL) {
		return;
	}

	if (opt->argdescr != NULL) {
		free(opt->argdescr);
	}
	if (opt->descr != NULL) {
		free(opt->descr);
	}

	name = opt->head;
	while (name) {
		next = name->next;
		free(name->name);
		free(name);
		name = next;
	}
	free(opt);

}

/* ========================================================================= */
/* vi:set ts=4 sw=4: */
