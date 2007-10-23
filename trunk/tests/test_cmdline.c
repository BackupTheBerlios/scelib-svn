#include <scelib/cmdline.h>
#include <scelib/platform.h>
#include <stdio.h>
#include <string.h>

struct opt
{
	char sname;
	char *lname;
	cmdline_cb_t cb;
	int pred;
};

struct test
{
	char *info;
	struct opt *opts;
	char **argv;
	int flags;
	int shown;
};

static char *gnu_argv[] =
{
	NULL,
	"-o",
	"-a",  "the a argument",
	"--long",
	"--arg=the arg argument",
	"-n",
	"--not-an-option",
	"--daemon",
	"--install=not recognized",
	"--",
	"-e", "end mark test",
	"-bad-long-style",
	"--bad-argument", "the bad argument",
	"/bad-win-style",
	NULL
};

static char *win_argv[] =
{
	NULL,
	"/o",
	"/a", "the a argument",
	"/long",
	"/arg", "the arg argument",
	"/n",
	"/not-an-option",
	"/install",
	"/daemon", "not recognized",
	"--",
	"/e", "end mark test",
	"-b",
	"-bad-java-style",
	"--bad-long-style",
	"/bad-argument=the bad argument",
	NULL
};

static char *java_argv[] =
{
	NULL,
	"-o",
	"-a", "the a argument",
	"-long",
	"-arg", "the arg argument",
	"-n",
	"-not-an-option",
	"-daemon",
	"-install",
	"--",
	"-e", "end mark test",
	"/b",
	"--bad-long-style",
	"-b=the bad argument",
	"-bad-argument=again the bad argument",
	NULL
};

static char *custom_argv[] =
{
	NULL,
	"-long",
	"-arg", "the arg argument",
	"-equal=the equal argument",
	"/not-an-option",
	"-daemon",
	"/install",
	"--",
	"-end", "end mark test",
	"/n",
	"-b",
	"--bad-long-style",
	"-b=the bad argument",
	NULL
};

static struct opt opts[] =
{
	{ 'o', "long", NULL, -1 },
	{ 'a', "arg", NULL, -1 },
	{ 'n', NULL, NULL, -1 },
	{ 0, "install", NULL, PLATFORM_IS(WINDOWS) },
	{ 0, "remove", NULL, PLATFORM_IS(WINDOWS) },
	{ 0, "daemon", NULL, PLATFORM_IS(UNIX) },
	{ 0, NULL, NULL, -1 }
};

static int callback(cmdline_t cl, cmdparsed_t *parsed, void *userdata)
{
	struct test *t = (struct test *) userdata;
	if (t && !t->shown)
	{
		puts("--------------------------------------------------------------------------");
		puts(t->info);
		puts("");
		t->shown = 1;
	}

	printf("[%2d]", parsed->opt_pos);
	if (parsed->opt_name)
		printf(" option '%s'", parsed->opt_name);
	if (parsed->opt_arg)
		printf(" '%s'", parsed->opt_arg);
	if (parsed->optid != -1)
		printf(" [%d]", parsed->optid);
	puts("");
	return 0;
}

static int test(char *progname, struct test *t)
{
	cmdline_t cl;
	int argc;

	cl = cmdline_create(t->flags, callback);
	if (cl == NULL)
		return 1;

	if (t->opts)
	{
		struct opt *o = t->opts;
		do
		{
			if (o->sname == 0 && o->lname == NULL && o->cb == NULL && o->pred == -1)
				break;
			if (o->pred == -1)
				cmdline_addopt(cl, o->sname, o->lname, o->cb);
			else
				cmdline_addopt_if(cl, o->sname, o->lname, o->cb, o->pred);
		}
		while (++o);
	}

	t->argv[0] = strrchr(progname, PLATFORM_DIRSEPC);
	if (t->argv[0] == NULL)
		t->argv[0] = progname;
	else
		++ (t->argv[0]);

	puts("");
	for (argc = 0; t->argv[argc] != NULL; ++argc)
		printf("%s ", t->argv[argc]);
	puts("");
	puts("");

	t->shown = 0;
	argc = cmdline_parse(cl, argc, t->argv, t);

	cmdline_destroy(cl);
	return argc;
}

int main(int argc, char **argv)
{
	struct test t;
	char *which = (argc > 1 ? argv[1] : NULL);

	if (!which || !strcmp(which, "gnu"))
	{
		/* simulate Unix options */
		opts[3].pred = 0;
		opts[4].pred = 0;
		opts[5].pred = 1;

		t.info = "GNU style without end mark";
		t.argv = gnu_argv;
		t.opts = opts;
		t.flags = CMDF_GNUSTYLE;
		if (test(argv[0], &t) != 0)
			return 1;

		t.info = "GNU style with end mark";
		t.flags |= CMDF_ENDMARK;
		if (test(argv[0], &t) != 0)
			return 1;

		t.info = "GNU style without end mark, only defined options";
		t.flags |= CMDF_ONLYDEFS;
		t.flags &= ~CMDF_ENDMARK;
		if (test(argv[0], &t) != 0)
			return 1;
	}

	if (!which || !strcmp(which, "win"))
	{
		/* simulate Windows options */
		opts[3].pred = 1;
		opts[4].pred = 1;
		opts[5].pred = 0;

		t.info = "Windows style without end mark";
		t.argv = win_argv;
		t.opts = opts;
		t.flags = CMDF_WINSTYLE;
		if (test(argv[0], &t) != 0)
			return 1;

		t.info = "Windows style with end mark";
		t.flags |= CMDF_ENDMARK;
		if (test(argv[0], &t) != 0)
			return 1;

		t.info = "Windows style without end mark, only defined options";
		t.flags |= CMDF_ONLYDEFS;
		t.flags &= ~CMDF_ENDMARK;
		if (test(argv[0], &t) != 0)
			return 1;
	}

	if (!which || !strcmp(which, "java"))
	{
		/* remove this predicated stuff */
		opts[3].pred = 0;
		opts[4].pred = 0;
		opts[5].pred = 0;

		t.info = "Java style without end mark";
		t.argv = java_argv;
		t.opts = opts;
		t.flags = CMDF_JAVASTYLE;
		if (test(argv[0], &t) != 0)
			return 1;

		t.info = "Java style with end mark";
		t.flags |= CMDF_ENDMARK;
		if (test(argv[0], &t) != 0)
			return 1;

		t.info = "Java style without end mark, only defined options";
		t.flags |= CMDF_ONLYDEFS;
		t.flags &= ~CMDF_ENDMARK;
		if (test(argv[0], &t) != 0)
			return 1;
	}

	if (!which || !strcmp(which, "custom"))
	{
		/* remove this predicated stuff */
		opts[3].pred = 0;
		opts[4].pred = 0;
		opts[5].pred = 0;

		t.info = "Custom style without end mark";
		t.argv = custom_argv;
		t.opts = opts;
		t.flags = CMDF_DASHLONG|CMDF_SLASHLONG|CMDF_SPACELONG|CMDF_EQUALLONG;
		if (test(argv[0], &t) != 0)
			return 1;

		t.info = "Custom style with end mark";
		t.flags |= CMDF_ENDMARK;
		if (test(argv[0], &t) != 0)
			return 1;

		t.info = "Custom style without end mark, only defined options";
		t.flags |= CMDF_ONLYDEFS;
		t.flags &= ~CMDF_ENDMARK;
		if (test(argv[0], &t) != 0)
			return 1;
	}

	return 0;
}
