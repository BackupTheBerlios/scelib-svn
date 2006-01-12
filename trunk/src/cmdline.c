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
