/*	scelib - Simple C Extension Library
 *  Copyright (C) 2005-2007 Richard 'riri' GILL <richard@houbathecat.info>
 *
 *  cmdline.c - command line handling functions.
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

#include "scelib/cmdline.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define FLAG(c, f)	(c->flags & (f))

typedef struct cmdopt_type cmdopt_t;

struct cmdline_type
{
	int flags;
	cmdline_cb_t cb;

	cmdopt_t *opthead;
	int optcount;
};

struct cmdopt_type
{
	int id;
	char sname;
	char *lname;
	cmdline_cb_t cb;

	cmdopt_t *next;
};

/* ------------------------------------------------------------------------- */
static void cmdline_fillarg(cmdparsed_t *arg, char *argval)
{
	arg->opt_name = NULL;
	arg->opt_arg = argval;
}

/* ------------------------------------------------------------------------- */
static int cmdline_isoption(cmdline_t cl, char *arg)
{
	int len = (int) strlen(arg);
	char *ptr = strchr(arg, '=');
	if (ptr)
		len = (int) (ptr - arg);

	if (arg[0] == '-')
	{
		--len;
		if (arg[1] == '-')
		{
			--len;
			if (FLAG(cl, CMDF_DDASHLONG) && len > 1)
			{
				if ((ptr && FLAG(cl, CMDF_EQUALLONG)) ||
					(!ptr && FLAG(cl, CMDF_SPACELONG)))
					return 2;
			}
		}
		else if ((FLAG(cl, CMDF_DASHSHORT) && len == 1) ||
			(FLAG(cl, CMDF_DASHLONG) && len > 1))
		{
			if (ptr)
			{
				if ((FLAG(cl, CMDF_EQUALSHORT) && len == 1) ||
					(FLAG(cl, CMDF_EQUALLONG) && len > 1))
					return 1;
			}
			else
			{
				if ((FLAG(cl, CMDF_SPACESHORT) && len == 1) ||
					(FLAG(cl, CMDF_SPACELONG) && len > 1))
					return 1;
			}
		}
	}
	else if (arg[0] == '/')
	{
		if ((FLAG(cl, CMDF_SLASHSHORT) && len == 1) ||
			(FLAG(cl, CMDF_SLASHLONG) && len > 1))
		{
			if (ptr)
			{
				if ((FLAG(cl, CMDF_EQUALSHORT) && len == 1) ||
					(FLAG(cl, CMDF_EQUALLONG) && len > 1))
					return 1;
			}
			else
			{
				if ((FLAG(cl, CMDF_SPACESHORT) && len == 1) ||
					(FLAG(cl, CMDF_SPACELONG) && len > 1))
					return 1;
			}
		}
	}
	return 0;
}

/* ------------------------------------------------------------------------- */
static int cmdline_isendmark(cmdline_t cl, char *arg)
{
	return (!strcmp(arg, "--") && FLAG(cl, CMDF_ENDMARK) ? 1 : 0);
}

/* ------------------------------------------------------------------------- */
static cmdopt_t *cmdline_checkdefined(cmdline_t cl, cmdparsed_t *current)
{
	cmdopt_t *opt;
	int len = (int) strlen(current->opt_name);

	opt = cl->opthead;
	while (opt)
	{
		if ((len == 1 && opt->sname == current->opt_name[0]) ||
			(len > 1 && !strcmp(opt->lname, current->opt_name)))
		{
			return opt;
		}
		opt = opt->next;
	}
	return NULL;
}

/* ------------------------------------------------------------------------- */
cmdline_t cmdline_create(int flags, cmdline_cb_t callback)
{
	cmdline_t cl;

	if (callback == NULL)
		return RETERROR(EINVAL, NULL);

	if ((cl = (cmdline_t) malloc(sizeof(struct cmdline_type))) == NULL)
		return NULL;

	cl->flags = flags;
	cl->cb = callback;
	cl->opthead = NULL;
	cl->optcount = 0;

	return cl;
}

/* ------------------------------------------------------------------------- */
int cmdline_destroy(cmdline_t cl)
{
	cmdopt_t *ptr, *next;

	if (cl == NULL)
		return RETERROR(EINVAL, -1);

	ptr = cl->opthead;
	while (ptr)
	{
		next = ptr->next;
		free(ptr);
		ptr = next;
	}
	free(cl);
	return 0;
}

/* ------------------------------------------------------------------------- */
int cmdline_parse(cmdline_t cl, int argc, char **argv, void* userdata)
{
	int argi, cur, start, optlen;
	int ret = 0;		/* return from callback */
	int endopt = 0;		/* indicates end of options */
	cmdparsed_t got;	/* current parsed option */
	cmdopt_t *opt;		/* reference to defined option */
	cmdline_cb_t cb;	/* global or option callback */

	if (cl == NULL)
		return RETERROR(EINVAL, -1);

	/* if no argument, just return */
	if (argc == 1)
		return 0;

	/* for each argument passed (non options are skipped) */
	got.opt_name = NULL;
	for (argi = 1, cur = argi; argi < argc; ++argi, cur = argi)
	{
		/* set the argument position */
		got.opt_pos = argi;
		if (got.opt_name)
		{
			free(got.opt_name);
			got.opt_name = NULL;
		}
		cb = NULL;

		if (endopt)
		{
			/* don't bother with options, just fill a simple argument */
			cmdline_fillarg(&got, argv[argi]);
			if ((ret = cl->cb(cl, &got, userdata)) != 0)
				break;
			continue;
		}

		/* -- alone, mark the end of options */
		if (!strcmp(argv[argi], "--"))
		{
			if (FLAG(cl, CMDF_ENDMARK))
			{
				endopt = 1;
				continue;
			}
			cmdline_fillarg(&got, argv[argi]);
			if (FLAG(cl, CMDF_ONLYDEFS) &&
				(ret = cl->cb(cl, &got, userdata)) != 0)
				break;
			continue;
		}

		/* - alone, not really an option */
		if (!strcmp(argv[argi], "-"))
		{
			cmdline_fillarg(&got, argv[argi]);
			if (FLAG(cl, CMDF_ONLYDEFS) &&
				(ret = cl->cb(cl, &got, userdata)) != 0)
				break;
			continue;
		}

		/* check if argument with correct style */
		start = cmdline_isoption(cl, argv[argi]);
		if (start > 0)
		{
			got.opt_arg = strchr(argv[argi], '=');
			if (got.opt_arg)
			{
				got.opt_arg = ++got.opt_arg;
				optlen = (int) (got.opt_arg - argv[argi]) - 1 - start;
			}
			else
			{
				optlen = (int) strlen(argv[argi] + start);
				if (argi < argc - 1 &&
					cmdline_isoption(cl, argv[argi + 1]) == 0 &&
					!cmdline_isendmark(cl, argv[argi + 1]))
					got.opt_arg = argv[++argi];
			}

			got.opt_name = (char *) malloc(sizeof(char) * (optlen + 1));
			if (got.opt_name == NULL)
				return -1;
			strncpy(got.opt_name, argv[cur] + start, optlen);
			got.opt_name[optlen] = '\0';
		}
		else
		{
			cmdline_fillarg(&got, argv[argi]);
		}

		/* check for defined options */
		if (got.opt_name)
		{
			opt = cmdline_checkdefined(cl, &got);
			if (opt)
			{
				got.optid = opt->id;
				if (opt->cb)
					cb = opt->cb;
			}
			else
			{
				if (FLAG(cl, CMDF_ONLYDEFS))
					continue;
				got.optid = -1;
			}
		}
		if (!cb)
			cb = cl->cb;
		if ((ret = cb(cl, &got, userdata)) != 0)
			break;
	}

	if (got.opt_name)
		free(got.opt_name);

	return ret;
}

/* ------------------------------------------------------------------------- */
int cmdline_addopt(cmdline_t cl, char sname, char *lname,
				   cmdline_cb_t callback)
{
	cmdopt_t *newopt;

	if ((newopt = (cmdopt_t *) malloc(sizeof(cmdopt_t))) == NULL)
		return -1;

	newopt->id = cl->optcount + 1;
	newopt->sname = sname;
	newopt->lname = lname;
	newopt->cb = callback;
	newopt->next = cl->opthead;
	cl->opthead = newopt;

	return ++ cl->optcount;
}

/* ------------------------------------------------------------------------- */
int cmdline_addopt_if(cmdline_t cl, char sname, char *lname,
					  cmdline_cb_t callback, int pred)
{
	if (pred)
		return cmdline_addopt(cl, sname, lname, callback);
	return -1;
}

/* ------------------------------------------------------------------------- */
int cmdline_getopt(cmdline_t cl, int optid, char *sname, char **lname)
{
	cmdopt_t *opt;
	if (!cl || optid < 0 || optid > cl->optcount)
		return RETERROR(EINVAL, -1);

	opt = cl->opthead;
	while (opt)
	{
		if (opt->id == optid)
		{
			*sname = opt->sname;
			*lname = opt->lname;
			return 0;
		}
	}
	return RETERROR(EINVAL, -1);
}

/* vi:set ts=4 sw=4: */
