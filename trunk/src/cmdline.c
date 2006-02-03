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

#include "scelib.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

/* ========================================================================= */
/* Types                                                                     */

typedef struct namelist_tag {	/* list of names */
	char *name;
	size_t len;
	int style;			/* command line style */
} namelist_;

/* ------------------------------------------------------------------------- */
/* type option_                                                              */
typedef struct option_tag {

	namelist_ *namelist;
	size_t namec;		/* number of names */

	int argreq;			/* argument requirement */

} option_;

/* ------------------------------------------------------------------------- */
/* type cmdline_t                                                            */
struct cmdline_ {

	int grow;		/* grow by for reallocation */

	option_ *opts;
	int count;
	int size;

	int trashs;		/* size of trashv array */
	int trashc;		/* number of elements in trash array */
	char **trashv;	/* array of trashed options/arguments */
};

/* ========================================================================= */
/* Static functions declaration                                              */

static int cmd_new_option(cmdline_t cmd);
static int cmd_add_trash(cmdline_t cmd, char *argv);

/* ========================================================================= */
/* Public functions definitions                                              */

cmdline_t cmd_create(int size, int grow) {
	cmdline_t cmd;

	if ((cmd = mem_new(struct cmdline_, 1)) != NULL) {
		if (size > 0) {
			cmd->size = size;
			cmd->opts = mem_new(option_, cmd->size);
			if (cmd->opts == NULL) {
				free(cmd);
				cmd = NULL;
			}
			else {
				cmd->grow = (grow > 0) ? grow : 1;
			}
		}
	}
	return cmd;
}

void cmd_destroy(cmdline_t cmd) {
	if (cmd == NULL) {
		errno = EINVAL;
		return;
	}

	/* if any options allocated */
	if (cmd->size > 0 && cmd->opts != NULL) {
		/* if any options defined */
		if (cmd->count > 0) {
			int i, j;
			option_ *opt;
			/* for each option */
			for (i = 0; i < cmd->count; ++i) {
				opt = cmd->opts+i;
				/* free all names */
				for (j = 0; j < opt->namec; ++j) {
					free(opt->namelist[j].name);
				}
				free(opt->namelist);
			}
			/* free the allocated options */
			free(cmd->opts);
		}
	}
	free(cmd);
}

int cmd_option_add(cmdline_t cmd, char *name, int style, int argreq) {
	int i;
	option_ *opt;

	if (cmd == NULL) {
		errno = EINVAL;
		return -1;
	}

	/* handle options array allocation */
	if ((i = cmd_new_option(cmd)) < 0) {
		return -1;
	}
	opt = cmd->opts+i;

	/* handle option names array */
	if ((opt->namelist = mem_new(namelist_, 1)) == NULL) {
		return -1;
	}
	if ((opt->namelist->name = strdup(name)) == NULL) {
		int err = errno;
		free(opt->namelist);
		opt->namelist = NULL;
		errno = err;
		return -1;
	}
	opt->namelist->len = strlen(name);
	opt->namelist->style = style;
	opt->namec ++;
	opt->argreq = argreq;

	/* return the index of the new option */
	return cmd->count ++;
}

int cmd_option_add_alt(cmdline_t cmd, int idx, char *altname, int style) {
	option_ *opt;
	namelist_ *newlist;

	if (cmd == NULL || idx >= cmd->count) {
		errno = EINVAL;
		return -1;
	}
	opt = cmd->opts+idx;

	if ((newlist = mem_new(namelist_, opt->namec + 1)) == NULL) {
		return -1;
	}
	if ((newlist[opt->namec].name = strdup(altname)) == NULL) {
		free(newlist);
		return -1;
	}
	newlist[opt->namec].len = strlen(altname);
	newlist[opt->namec].style = style;
	memcpy(newlist, opt->namelist, sizeof(namelist_) * opt->namec);
	free(opt->namelist);
	opt->namelist = newlist;
	opt->namec ++;
	return opt->namec;
}

int cmd_find(cmdline_t cmd, int idx, char **name, char **arg) {
	return 0;
}

int cmd_getnext(cmdline_t cmd, cmdparsed_t *parsed) {
	return 0;
}

int cmd_loop(cmdline_t cmd, char **arg) {
	return 0;
}

int cmd_reset(cmdline_t cmd) {
	return 0;
}

/* ========================================================================= */
/* Static functions definition                                               */

static int cmd_new_option(cmdline_t cmd) {
	if (cmd->size == 0) {
		if ((cmd->opts = mem_new(option_, cmd->grow)) != NULL) {
			cmd->size = cmd->grow;
		}
	}
	else if (cmd->count == cmd->size) {
		option_ *opts = mem_new(option_, cmd->size + cmd->grow);
		if (opts != NULL) {
			memcpy(opts, cmd->opts, sizeof(option_) * cmd->count);
			free(cmd->opts);
			cmd->opts = opts;
			cmd->size += cmd->grow;
		}
	}

	return (cmd->opts == NULL) ? -1 : 0;
}

static int cmd_add_trash(cmdline_t cmd, char *argv) {
	if (cmd->trashs == 0) {
		if ((cmd->trashv = mem_new(char *, cmd->grow)) == NULL) {
			return -1;
		}
		cmd->trashs = cmd->grow;
	}
	else if (cmd->trashc == cmd->trashs) {
		char **args;
		if ((args = mem_new(char *, cmd->trashs + cmd->grow)) == NULL) {
			return -1;
		}
		memcpy(args, cmd->trashv, sizeof(char *) * cmd->trashc);
		free(cmd->trashv);
		cmd->trashv = args;
	}
	cmd->trashv[cmd->trashc] = argv;
	cmd->trashc ++;
	return 0;
}



/* ######################################################################### */
/* old code                                                                  */
/* ######################################################################### */
#if 0

/* ========================================================================= */
/* Static functions declaration                                              */

static int cmd_findtrash(cmdopt_t *opttab, int optcount);
static void cmd_addtrash(cmdopt_t *opttab, int trashidx, char *option);
static int cmd_checkoption(cmdopt_t *opttab, int optcount,
	int argc, char **argv, int current);



/* ========================================================================= */
/* Public functions definitions                                              */

/* ------------------------------------------------------------------------- */
/* cmdline_parse()                                                           */

int cmdline_parse(int argc, char **argv, cmdopt_t *opttab, int optcount) {

	int idx, trashopt;
	int ret = 0;

	/* first, lets find a special trash element in the array */
	trashopt = cmd_findtrash(opttab, optcount);

	/* initialize the optidx to 'not found' */
	for (idx = 0; idx < optcount; ++idx) {
		opttab[idx].optidx = CMDERR_NOSPEC;
	}

	/* for each argument passed (non options are skipped) */
	idx = 0;
	while (idx < argc) {
		if (argv[idx][0] != '-' || argv[idx][1] == '\0') {
			/* not an option, add to 'trash' option if present */
			cmd_addtrash(opttab, trashopt, argv[idx]);
		}
		/* do the work for this one */
		idx = cmd_checkoption(opttab, optcount, argc, argv, idx);
		/* idx has been incremented as needed to the next */
	}

	/* check if at least one of the options is specified and in error */
	for (idx = 0; idx < optcount; ++idx) {
		if (opttab[idx].optidx != CMDERR_NOSPEC &&
			opttab[idx].error != CMDERR_OK) {
			ret = -1;
			break;
		}
	}
	return ret;

}



/* ========================================================================= */
/* Static functions definition                                               */

/* ------------------------------------------------------------------------- */
/* cmd_findtrash()                                                           */
/* Check in the options array if there's a special element (zeroed), and     */
/* returns the index to it, returns -1 otherwise.                            */

static int cmd_findtrash(cmdopt_t *opttab, int optcount) {

	int idx = 0;

	/* scan the array for a special element */
	while (idx < optcount) {
		if (opttab[idx].style == 0 && opttab[idx].eqstyle == 0 &&
			opttab[idx].argreq == 0 && opttab[idx].opt == NULL) {
			return idx;
		}
		++idx;
	}

	/* no trash element */
	return -1;

}

/* ------------------------------------------------------------------------- */
/* cmd_addtrash()                                                            */
/* Appends the specified option in the trash, if there's one.                */

static void cmd_addtrash(cmdopt_t *opttab, int trashidx, char *option) {

	/* no trash index, do not conserve not expected options */
	if (trashidx == -1) {
		return;
	}

}

/* ------------------------------------------------------------------------- */
/* cmd_checkoption()                                                         */
/* Does the work of finding an option corresponding to the command line      */
/* argument (indexed by 'current'), and filling the found option with right  */
/* values. Returns the updated command line argument counter to let the      */
/* caller verifying the next unchecked argument.                             */

static int cmd_checkoption(cmdopt_t *opttab, int optcount,
						   int argc, char **argv, int current) {

	int start = 1, found = -1;
	int idx, optlen, len;
	char *thisarg, *ptrarg;
	char optstr[CMDOPT_NAMELEN+1];

	/* we know the first character is a '-', skip it */
	thisarg = argv[current] + 1;
	len = (int) strlen(thisarg);	/* remove first '-' */
	optlen = len;	/* defaut to only arg */

	/* first try to determine the style of the option, and the presence of an
	   argument with '=' */
	if (*thisarg == '-') {
		/* double dash ('--'), start one character right */
		++thisarg;
		++start;
		--len;
		--optlen;
	}

	/* try to find an equal sign */
	ptrarg = strchr(thisarg, '=');
	if (ptrarg != NULL) {
		/* '=' specified */
		++ptrarg;
		optlen = (int) (ptrarg - thisarg) - 1;
	}
	/* in all cases, copy the option name in a temporary buffer */
	strncpy(optstr, thisarg, optlen);
	optstr[optlen] = '\0';

	/* now optstr[] contains the option, and *ptrarg the argument if present
	   (NULL otherwise). If double dash ('--'), start=2, start=1 otherwise.
	   optlen contains the length of the option, so optlen=1 means a short one
	*/
	for (idx = 0; idx < optcount; ++idx) {
		if (opttab[idx].opt == NULL) {
			continue;	/* this's the trash */
		}

		/* if option name and extracted one match */
		if (!strcmp(optstr, opttab[idx].opt)) {
			/* check the option style (short or long) */
			if (opttab[idx].style == CMDFLAG_SHORT && start != 1) {
				continue;	/* error, '-' expected, '--' found */
			}
			if (opttab[idx].style == CMDFLAG_LONG && start != 2) {
				continue;	/* error, '--' expected, '-' found */
			}
			/* save informations */
			opttab[idx].optidx = current;	/* option recognized */
			opttab[idx].arg = ptrarg;	/* the argument or NULL */
			found = idx;
			break;
		}
	}
	if (found != -1) {	/* found an option */
		opttab[found].error = CMDERR_OK;	/* be positive :-) */

		/* if argument specified with an equal sign (arg already checked) */
		if (opttab[found].arg != NULL) {
			/* if no equal sign expected, report the error */
			if (opttab[found].eqstyle == CMDFLAG_NOEQUAL) {
				opttab[found].error = CMDERR_EQUAL;
			}
		}
		else {
			/*
			 * no argument yet, try to find one in the next command line
			 * argument
			 */
			if (argv[current + 1] != NULL && argv[current + 1][0] != '-') {
				/* next is not an option, put that as an argument */
				opttab[found].arg = argv[current + 1];
				++current;	/* pass this next argument */

				/* if an equal sign was expected (not the case here), report */
				if (opttab[found].eqstyle == CMDFLAG_EQUAL) {
					opttab[found].error = CMDERR_EQUAL;
				}
			}
		}

		/* now if argument supplied but not wanted, report the error */
		if (opttab[found].arg != NULL) {
			if (opttab[found].argreq == CMDARG_NONE) {
				opttab[found].error = CMDERR_ARG;
			}
		}
		else {
			/* but if wanted, we should have one */
			if (opttab[found].argreq == CMDARG_REQ) {
				opttab[found].error = CMDERR_ARG;
			}
		}
	}

	/* go to next command line argument */
	return ++current;

}

#endif

/* ========================================================================= */
/* vi:set ts=4 sw=4: */
