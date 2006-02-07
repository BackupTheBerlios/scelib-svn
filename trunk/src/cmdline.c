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
/* Constants                                                                 */

/* default grow size for new options in the cmdline_t.opts array */
#define CMDOPT_GROWSIZE		10

/* grow size for new names in the cmdopt_t.names array */
#define CMDNAME_GROWSIZE	3

/* ========================================================================= */
/* Types                                                                     */

/* ------------------------------------------------------------------------- */
/* type cmdname_t                                                            */
/* Quick helper to store option names (first and alternates) with their size */
/* and the style for argument separation.                                    */

typedef struct cmdname_ {

	char *name;			/* option name */
	int len;			/* length of name */
	int style;			/* argument separator style */

} cmdname_t;

/* ------------------------------------------------------------------------- */
/* type cmdopt_t                                                             */
/* Defines one option: its name(s), the number of defined names, the size of */
/* the names array growing by CMDNAME_GROWSIZE (for performance issues), the */
/* argument requirement flag, the description and argument text for help.    */

typedef struct cmdopt_ {

	cmdname_t *names;	/* array of names */
	int namec;			/* number of names */
	int nsize;			/* size of names array */
	int argreq;			/* argument requirement flag */
	char *descr;		/* option description (for help) */
	char *argdescr;		/* argument description (for help) */

} *cmdopt_t;

/* ------------------------------------------------------------------------- */
/* type cmdline_t                                                            */
/* Overall structure to manage command line options.                         */
/* The opts/optc/osize triplet acts like the names/namec/nsize one in the    */
/* cmdopt_ structure. The grow size can be specified at tool creation, or    */
/* will have the CMDOPT_GROWSIZE default value.                              */
/* argc and argv are stored are tool creation to simplify parser functions.  */

struct cmdline_ {

	cmdopt_t *opts;		/* array of options */
	int optc;			/* number of options */
	int osize;			/* size of options array */

	int grow;			/* options array growing number */

	int argc;			/* saved argc */
	char **argv;		/* saved argv */

};



/* ========================================================================= */
/* Static functions declaration                                              */

/* ------------------------------------------------------------------------- */
/* cmd_opt_new()                                                             */
/* Creates a new cmdopt_ structure.                                          */

static cmdopt_t cmd_opt_new(char *name, int style, int argreq, char *descr,
	char *argdescr);

/* ------------------------------------------------------------------------- */
/* cmd_opt_newname()                                                         */
/* Adds an alternate name to the specified option, managing memory           */
/* reallocation if needed.                                                   */

static int cmd_opt_newname(cmdopt_t opt, char *altname, int altstyle);

/* ------------------------------------------------------------------------- */
/* cmd_opt_del()                                                             */
/* Frees the specified option, with all allocated memory during option life. */

static void cmd_optdel(cmdopt_t opt);



/* ========================================================================= */
/* Public functions definitions                                              */

/* ------------------------------------------------------------------------- */
/* cmd_create()                                                              */

cmdline_t cmd_create(int argc, char **argv) {
	return cmd_create_ex(argc, argv, CMDOPT_GROWSIZE);
}

/* ------------------------------------------------------------------------- */
/* cmd_create_ex()                                                           */

cmdline_t cmd_create_ex(int argc, char **argv, int grow) {

	cmdline_t cmd;

	if (grow < 1) {
		errno = EINVAL;
		return NULL;
	}

	cmd = (cmdline_t) calloc(1, sizeof(struct cmdline_));
	if (cmd == NULL) {
		return NULL;
	}

	cmd->opts = (cmdopt_t *) calloc(grow, sizeof(cmdopt_t));
	if (cmd->opts == NULL) {
		int saved = errno;
		free(cmd);
		errno = saved;
		return NULL;
	}
	cmd->osize = grow;
	cmd->grow = grow;

	cmd->argc = argc;
	cmd->argv = argv;

	return cmd;

}

/* ------------------------------------------------------------------------- */
/* cmd_destroy()                                                             */

void cmd_destroy(cmdline_t cmd) {

	if (cmd == NULL) {
		return;
	}

	if (cmd->osize > 0) {
		int o;
		for (o = 0; o < cmd->optc; ++o) {
			cmd_optdel(cmd->opts[o]);
		}
		free(cmd->opts);
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

	if (cmd == NULL) {
		errno = EINVAL;
		return -1;
	}

	if (cmd->optc == cmd->osize) {
		cmdopt_t *newopts = (cmdopt_t *) calloc(cmd->osize + cmd->grow,
			sizeof(cmdopt_t));
		if (newopts == NULL) {
			return -1;
		}
		memcpy(newopts, cmd->opts, cmd->optc * sizeof(cmdopt_t));
		free(cmd->opts);
		cmd->opts = newopts;
	}

	cmd->opts[cmd->optc] = cmd_opt_new(name, style, argreq, descr, argdescr);
	if (cmd->opts[cmd->optc] == NULL) {
		return -1;
	}

	return cmd->optc ++;
}

/* ------------------------------------------------------------------------- */
/* cmd_addopt_name()                                                         */

int cmd_addopt_name(cmdline_t cmd, int idx, char *altname, int altstyle) {

	if (cmd == NULL || idx >= cmd->optc || altname == NULL) {
		errno = EINVAL;
		return -1;
	}

	return cmd_opt_newname(cmd->opts[idx], altname, altstyle);
}



/* ------------------------------------------------------------------------- */
/* cmd_print()                                                               */

void cmd_print(FILE *fd, cmdline_t cmd, char *head, char *tail) {
	int o, n;
	cmdopt_t opt;
	static char defargd[] = "value";

	if (head) {
		fprintf(fd, "%s\n", head);
	}

	if (cmd == NULL || cmd->optc == 0) {
		return;
	}

	for (o = 0; o < cmd->optc; ++o) {
		opt = cmd->opts[o];
		for (n = 0; n < opt->namec; ++n) {
			if (opt->argreq == CMDARG_NONE) {
				fprintf(fd, "%s\n", opt->names[n].name);
			}
			else {
				char *adesc = (opt->argdescr != NULL) ?
					opt->argdescr : defargd;
				char *format = NULL;

				switch (opt->names[n].style) {
					case CMDSTYLE_NONE:
						format = "%s'%s'\n";
						break;
					case CMDSTYLE_SPACE:
						format = "%s '%s'\n";
						break;
					case CMDSTYLE_EQUAL:
						format = "%s='%s'\n";
				}
				fprintf(fd, format, opt->names[n].name, adesc);
			}
		}
		if (opt->descr != NULL) {
			fprintf(fd, "\t%s\n", opt->descr);
		}
		fprintf(fd, "\n");
	}

	if (tail) {
		fprintf(fd, "%s\n", tail);
	}

}

/* ========================================================================= */
/* Static functions definition                                               */

/* ------------------------------------------------------------------------- */
/* cmd_opt_new()                                                             */

static cmdopt_t cmd_opt_new(char *name, int style, int argreq, char *descr,
	char *argdescr) {

	cmdopt_t opt;
	int saved;

	if (name == NULL) {
		errno = EINVAL;
		return NULL;
	}

	opt = (cmdopt_t) calloc(1, sizeof(struct cmdopt_));
	if (opt == NULL) {
		return NULL;
	}

	opt->names = (cmdname_t *) calloc(CMDNAME_GROWSIZE, sizeof(cmdname_t));
	if (opt->names == NULL) {
		saved = errno;
		free(opt);
		errno = saved;
		return NULL;
	}

	opt->names->name = strdup(name);
	if (opt->names->name == NULL) {
		saved = errno;
		free(opt->names);
		free(opt);
		errno = saved;
		return NULL;
	}
	opt->names->len = (int) strlen(opt->names->name);
	opt->names->style = style;
	opt->nsize = CMDNAME_GROWSIZE;
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

static int cmd_opt_newname(cmdopt_t opt, char *altname, int altstyle) {

	cmdname_t *newname;

	if (opt == NULL || opt->names == NULL || opt->namec < 1 ||
		altname == NULL) {
		errno = EINVAL;
		return -1;
	}

	if (opt->namec == opt->nsize) {
		cmdname_t *newnames = (cmdname_t *)
			calloc(opt->nsize + CMDNAME_GROWSIZE, sizeof(cmdname_t));
		if (newnames == NULL) {
			return -1;
		}
		memcpy(newnames, opt->names, opt->namec * sizeof(cmdname_t));
		free(opt->names);
		opt->names = newnames;
		opt->nsize += CMDNAME_GROWSIZE;
	}
	newname = opt->names + opt->namec;

	newname->name = strdup(altname);
	if (newname->name == NULL) {
		return -1;
	}
	newname->len = (int) strlen(newname->name);
	newname->style = altstyle;

	return opt->namec ++;

}

/* ------------------------------------------------------------------------- */
/* cmd_opt_del()                                                             */

static void cmd_optdel(cmdopt_t opt) {

	int i;

	if (opt == NULL) {
		return;
	}

	if (opt->argdescr != NULL) {
		free(opt->argdescr);
	}
	if (opt->descr != NULL) {
		free(opt->descr);
	}

	for (i = 0; i < opt->namec; ++i) {
		free((opt->names + i)->name);
	}
	free(opt->names);
	free(opt);

}

/* ========================================================================= */
/* vi:set ts=4 sw=4: */
