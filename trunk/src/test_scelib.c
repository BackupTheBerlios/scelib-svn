/*	scelib - Simple C Extension Library
 *  Copyright (C) 2005-2007 Richard 'riri' GILL <richard@houbathecat.info>
 *
 *  test_scelib.c - library tests program.
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

#include "scelib.h"
#include <stdio.h>

/* ------------------------------------------------------------------------- */

static int test_cmdline(int argc, char **argv);
static int test_thread(void);

/* ------------------------------------------------------------------------- */

int main(int argc, char **argv)
{
	int ret = test_cmdline(argc, argv);
	if (ret)
		return 1;
	return 0;
}

/* ------------------------------------------------------------------------- */

static int cmd_cb(cmdline_t cl, cmdparsed_t *parsed, void *userdata)
{
	printf("[%2d]", parsed->opt_pos);
	if (parsed->opt_name)
		printf(": option '%s'", parsed->opt_name);
	if (parsed->opt_arg)
		printf(": %s", parsed->opt_arg);
	if (userdata)
		printf(" - userdata: %s", (char *) userdata);
	puts("");
	return 0;
}

static int cmd_test_cb(cmdline_t cl, cmdparsed_t *parsed, void *userdata)
{
	if (parsed->opt_arg)
	{
		if (!strcmp(parsed->opt_arg, "thread"))
			return test_thread();
	}
	return 0;
}

static int test_cmdline(int argc, char **argv)
{
	cmdline_t cl;
	int ret;

	cl = cmdline_create(CMDF_JAVASTYLE|CMDF_ONLYDEFS, cmd_cb);
	if (cl == NULL)
		return 1;

	cmdline_addopt(cl, 0, "test", cmd_test_cb);
	cmdline_addopt_if(cl, 0, "install", NULL, PLATFORM_IS(WINDOWS));
	cmdline_addopt_if(cl, 0, "remove", NULL, PLATFORM_IS(WINDOWS));
	cmdline_addopt_if(cl, 0, "name", NULL, PLATFORM_IS(WINDOWS));
	cmdline_addopt_if(cl, 0, "daemon", NULL, PLATFORM_IS(UNIX));
	cmdline_addopt(cl, 0, "start", NULL);
	cmdline_addopt(cl, 0, "stop", NULL);

	ret = cmdline_parse(cl, argc, argv, NULL);

	cmdline_destroy(cl);
	return ret;
}

/* ------------------------------------------------------------------------- */

static int test_thread(void)
{
	printf("thread test\n");
	return 0;
}

/* vi:set ts=4 sw=4: */
