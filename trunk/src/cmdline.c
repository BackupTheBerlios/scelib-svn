/*
 *  scelib - Simpliest C Extension Library
 *  Copyright (C) 2005-2006 Richard 'riri' GILL <richard@houbathecat.info>
 *
 *  cmdline.c - cool command line parser.
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

#include "scelib.h"
#include <string.h>



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
	e_argsep argsep;	/* argument separator style */

	cmdname_t *next;	/* for linked list */

};

/* ------------------------------------------------------------------------- */
/* type cmdopt_t                                                             */
/* Defines one option: its name(s), the argument requirement flag, the       */
/* description and argument text for help.                                   */

typedef struct cmdopt_ cmdopt_t;
struct cmdopt_ {

	cmdname_t *nhead;	/* head of linked list of names */
	cmdname_t *ntail;	/* tail of linked list of names */
	int ncount;			/* number of names */
	cmdparse_cb cb;		/* callback */

	int argreq;			/* argument requirement flag */
	char *descr;		/* option description (for help) */
	char *argdescr;		/* argument description (for help) */

	cmdopt_t *next;		/* for linked list */

};

/* ------------------------------------------------------------------------- */
/* type cmdline_t                                                            */
/* Overall structure to manage command line options.                         */

struct cmdline_ {

	cmdopt_t *ohead;	/* head of linked list of options */
	cmdopt_t *otail;	/* tail of linked list of options */
	int ocount;			/* number of options */

	cmdparse_cb cb;		/* default callback */

};

/* ------------------------------------------------------------------------- */
/* type cmddata_t                                                            */
/* Handles temporary extracted command line information.                     */
/* The 'what' field defines if it's an option or an extracted argument (with */
/* the check of '='. 'argpos' reference the argv position.                   */
/* The 'data' is freed after usage if it's an option, but left as is in case */
/* of argument, because it's just a pointer to argv[x] after the '='.        */

#define AN_OPTION	1
#define AN_ARGUMENT	2

typedef struct cmddata_ cmddata_t;
struct cmddata_ {

	int what;			/* option or argument ? */
	int argpos;			/* argc position */
	char *data;			/* actual data (option name or argument */
	cmddata_t *next;	/* next in list */
	cmddata_t *prev;	/* previous in list */

};



/* ========================================================================= */
/* Static functions declaration                                              */

/* ------------------------------------------------------------------------- */
/* cmd_opt_new()                                                             */
/* Creates a new cmdopt_ structure.                                          */

static cmdopt_t *cmd_opt_new(char *name, e_argreq argreq, e_argsep argsep, char *descr, char *argdescr, cmdparse_cb cb);

/* ------------------------------------------------------------------------- */
/* cmd_opt_newname()                                                         */
/* Adds an alternate name to the specified option.                           */

static int cmd_opt_newname(cmdopt_t *opt, char *altname, e_argsep altsep);

/* ------------------------------------------------------------------------- */
/* cmd_opt_del()                                                             */
/* Frees the specified option, with all allocated memory during option life. */

static void cmd_opt_del(cmdopt_t *opt);

/* ------------------------------------------------------------------------- */
/* cmd_found_opt()                                                           */
/* Looks for an option which has the specified name.                         */

static int cmd_found_opt(cmdline_t cmd, char *name, cmdopt_t **popt, cmdname_t **pname, int *poid, int *pnid);

/* ------------------------------------------------------------------------- */
/* cmd_new_data()                                                            */
/* Allocates a new cmddata_t in the list, and update the current item to     */
/* this new one.                                                             */

static void cmd_new_data(cmddata_t **list, cmddata_t **cur);



/* ========================================================================= */
/* Public functions definitions                                              */

/* ------------------------------------------------------------------------- */
/* cmd_create()                                                              */

cmdline_t cmd_create(cmdparse_cb cb) {

	cmdline_t cmd = mem_new(struct cmdline_, 1);
	if (cmd)
		cmd->cb = cb;
	return cmd;

}

/* ------------------------------------------------------------------------- */
/* cmd_destroy()                                                             */

void cmd_destroy(cmdline_t cmd) {

	cmdopt_t *opt, *next;

	if (!cmd)
		return;

	/* free option list */
	opt = cmd->ohead;
	while (opt) {
		next = opt->next;
		cmd_opt_del(opt);
		opt = next;
	}
	free(cmd);

}

/* ------------------------------------------------------------------------- */
/* cmd_addopt()                                                              */

int cmd_addopt(cmdline_t cmd, char *name, char *descr, cmdparse_cb cb) {

	return cmd_addopt_arg(cmd, name, ARG_NONE, ARGSEP_NONE, descr, NULL, cb);

}

/* ------------------------------------------------------------------------- */
/* cmd_addopt_arg()                                                          */

int cmd_addopt_arg(cmdline_t cmd, char *name, e_argreq argreq, e_argsep argsep, char *descr, char *argdescr, cmdparse_cb cb) {

	cmdopt_t *newopt;

	if (!cmd || (argsep != ARGSEP_NONE && argreq == ARG_NONE) || !name || !cb)
		return reterror(EINVAL, -1);

	newopt = cmd_opt_new(name, argreq, argsep, descr, argdescr, cb);
	if (!newopt)
		return -1;

	if (cmd->ohead) {
		cmd->otail->next = newopt;
		cmd->otail = newopt;
	}
	else {
		cmd->ohead = newopt;
		cmd->otail = newopt;
	}

	return cmd->ocount ++;

}

/* ------------------------------------------------------------------------- */
/* cmd_addopt_name()                                                         */

int cmd_addopt_name(cmdline_t cmd, int optid, char *altname, e_argsep altsep) {

	cmdopt_t *opt;

	if (!cmd || optid >= cmd->ocount || !altname || !strlen(altname))
		return reterror(EINVAL, -1);

	opt = cmd->ohead;
	while (optid --)
		opt = opt->next;

	if (!opt->nhead->len ||
		(altsep == ARGSEP_NONE && opt->argreq != ARG_NONE) ||
		(altsep != ARGSEP_NONE && opt->argreq == ARG_NONE))
		return reterror(EINVAL, -1);

	return cmd_opt_newname(opt, altname, altsep);

}

/* ------------------------------------------------------------------------- */
/* cmd_getopt()                                                              */

int cmd_getopt(cmdline_t cmd, int optid, int nameid, char **name, e_argreq *argreq, e_argsep *argsep, char **descr, char **argdescr) {

	cmdopt_t *popt;
	cmdname_t *pname;

	if (!cmd || optid >= cmd->ocount)
		return reterror(EINVAL, -1);

	popt = cmd->ohead;
	while (optid --)
		popt = popt->next;

	if (popt->ncount < nameid)
		return reterror(EINVAL, -1);

	pname = popt->nhead;
	while (nameid --)
		pname = pname->next;

	*name = pname->name;
	*argreq = popt->argreq;
	*argsep = pname->argsep;
	*descr = popt->descr;
	*argdescr = popt->argdescr;

	return 0;

}

/* ------------------------------------------------------------------------- */
/* cmd_print()                                                               */

void cmd_print(FILE *fd, cmdline_t cmd, char *head, char *tail) {

	static char defargd[] = "value";

	if (head) {
		fprintf(fd, "%s\n", head);
	}

	if (cmd && cmd->ocount > 0) {
		cmdname_t *pname;
		cmdopt_t *popt = cmd->ohead;
		char *adesc, format;

		while (popt) {
			if (!popt->descr) {
				popt = popt->next;
				continue;
			}

			pname = popt->nhead;
			while (pname) {
				if (popt->argreq == ARG_NONE) {
					fprintf(fd, "%s\n", pname->name);
				}
				else {
					adesc = (popt->argdescr) ? popt->argdescr : defargd;
					if (pname->argsep == ARGSEP_SPACE)
						format = ' ';
					else
						format = '=';
					fprintf(fd, "%s%c%s\n", pname->name, format, adesc);
				}
				pname = pname->next;
			}
			if (popt->nhead->len == 0)
				fprintf(fd, "%s\n", popt->descr);
			else
				fprintf(fd, "\t%s\n", popt->descr);
			popt = popt->next;
		}
	}

	if (tail) {
		fprintf(fd, "%s\n", tail);
	}

}

/* ------------------------------------------------------------------------- */
/* cmd_parse()                                                               */

int cmd_parse(cmdline_t cmd, int argc, char **argv, void *userdata) {
	int i, len, oid, nid;
	cmdopt_t *popt, *ptemp;
	cmdname_t *pname;
	cmdparse_cb cb;
	cmdparsed_t parsed;
	char *ptr;
	int ret = 0;
	cmddata_t *list = 0;
	cmddata_t *cur = 0;

	/* first generate a list of extracted command line arguments unchecked */
	for (i = 1; i < argc; ++i) {
		/* create a new data container  */
		cmd_new_data(&list, &cur);

		/* check if we have the form option=argument */
		ptr = strchr(argv[i], '=');
		if (ptr) {
			/* create another container for the argument */
			cmd_new_data(&list, &cur);

			/* store the argument */
			cur->data = ptr + 1;
			cur->what = AN_ARGUMENT;

			/* store the option */
			len = (int) (ptr - argv[i]);
			cur->prev->data = mem_new(char, len + 1);
			strncpy(cur->prev->data, argv[i], len);
			cur->prev->data[len] = '\0';
			cur->prev->what = AN_OPTION;
			cur->prev->argpos = i;
		}
		else {
			/* store the option */
			cur->data = strdup(argv[i]);
			cur->what = AN_OPTION;
			cur->argpos = i;
		}
	}

	/* now option checking is easier with our options list */
	for (cur = list; cur; cur = cur->next) {
		/* initialize the parse structure */
		mem_zero(&parsed, sizeof(cmdparsed_t));
		parsed.name = cur->data;
		parsed.argpos = cur->argpos;

		/* if another data and is an argument (with '='), initialize it */
		if (cur->next && cur->next->what == AN_ARGUMENT) {
			parsed.arg = cur->next->data;
			cur = cur->next;
		}
		if (cmd_found_opt(cmd, parsed.name, &popt, &pname, &oid, &nid)) {
			/* found a defined option, store option reference */
			parsed.optid = oid;
			parsed.nameid = nid;
			cb = popt->cb;

			if (parsed.arg) {
				/* we already have an '=' argument */
				if (popt->argreq == ARG_NONE) {
					parsed.error = CMDERR_REQ;	/* shouldn't be there */
				}
				else if (pname->argsep != ARGSEP_EQUAL) {
					parsed.error = CMDERR_SEP;	/* should be separated with a space */
				}
			}
			else {
				/* if the option may have an argument */
				if (popt->argreq != ARG_NONE) {
					if (pname->argsep == ARGSEP_EQUAL) {
						parsed.error = CMDERR_SEP;	/* should be separated with an equal sign */
					}
					/* check the next to see if it's an option */
					if (!cur->next) {
						parsed.error |= CMDERR_REQ;
					}
					else {
						if (cmd_found_opt(cmd, cur->next->data, &ptemp, &pname, &oid, &nid)) {
							/* argument is in fact an option ! */
							if (popt->argreq == ARG_REQUIRED)
								parsed.error |= CMDERR_REQ;	/* should have an argument */
						}
						else {
							/* next is considered as an argument */
							cur = cur->next;
							parsed.arg = cur->data;
						}
					}
				}
			}
			ret = cb(cmd, &parsed, userdata);
		}
		else {
			/* not a defined option */
			parsed.optid = -1;
			parsed.nameid = -1;
			parsed.error = 0;
			/* if default callback is defined in cmdline_t object */
			if (cmd->cb)
				ret = cmd->cb(cmd, &parsed, userdata);
		}

		/* if last called user function returned 1, stop */
		if (!ret)
			break;
	}

	/* free cmddata_t .. */
	while (list) {
		cur = list->next;
		if (list->what == AN_OPTION)	/* because argument is from argv */
			free(list->data);
		free(list);
		list = cur;
	}

	return ret;
}



/* ========================================================================= */
/* Static functions definition                                               */

/* ------------------------------------------------------------------------- */
/* cmd_opt_new()                                                             */

static cmdopt_t *cmd_opt_new(char *name, e_argreq argreq, int argsep, char *descr, char *argdescr, cmdparse_cb cb) {

	cmdopt_t *newopt;

	newopt = mem_new(cmdopt_t, 1);
	if (!newopt)
		return NULL;

	if (cmd_opt_newname(newopt, name, argsep) < 0)
		return NULL;

	newopt->cb = cb;
	newopt->argreq = argreq;

	if (descr)
		newopt->descr = strdup(descr);

	if (argdescr)
		newopt->argdescr = strdup(argdescr);

	return newopt;

}

/* ------------------------------------------------------------------------- */
/* cmd_opt_newname()                                                         */

static int cmd_opt_newname(cmdopt_t *opt, char *altname, int altsep) {

	cmdname_t *newname;
	int esaved;

	newname = mem_new(cmdname_t, 1);
	if (!newname)
		return -1;

	newname->name = strdup(altname);
	if (!newname) {
		esaved = errno;
		free(newname);
		reterror(esaved, -1);
	}

	newname->len = (int) strlen(altname);
	newname->argsep = altsep;

	if (opt->nhead) {
		opt->ntail->next = newname;
		opt->ntail = newname;
	}
	else {
		opt->nhead = newname;
		opt->ntail = newname;
	}

	return opt->ncount ++;

}

/* ------------------------------------------------------------------------- */
/* cmd_opt_del()                                                             */

static void cmd_opt_del(cmdopt_t *opt) {

	cmdname_t *name, *next;

	if (!opt)
		return;

	if (opt->argdescr)
		free(opt->argdescr);

	if (opt->descr)
		free(opt->descr);

	name = opt->nhead;
	while (name) {
		next = name->next;
		free(name->name);
		free(name);
		name = next;
	}
	free(opt);

}

/* ------------------------------------------------------------------------- */
/* cmd_found-opt()                                                           */

static int cmd_found_opt(cmdline_t cmd, char *name, cmdopt_t **popt, cmdname_t **pname, int *poid, int *pnid) {

	*popt = cmd->ohead;
	*poid = 0;
	*pnid = 0;
	while (*popt) {
		*pname = (*popt)->nhead;
		while (*pname) {
			if ((*pname)->name && !strcmp((*pname)->name, name))
				return 1;
			*pname = (*pname)->next;
			++ *pnid;
		}
		*popt = (*popt)->next;
		++ *poid;
		*pnid = 0;
	}
	*poid = -1;
	*pnid = -1;
	return 0;

}

/* ------------------------------------------------------------------------- */
/* cmd_new_data()                                                            */

static void cmd_new_data(cmddata_t **list, cmddata_t **cur) {

	cmddata_t *dt = mem_new(cmddata_t, 1);
	if (*cur) {
		(*cur)->next = dt;
		dt->prev = *cur;
		*cur = dt;
	}
	else {
		*list = dt;
		*cur = dt;
	}

}



/* ========================================================================= */
/* vi:set ts=4 sw=4: */
