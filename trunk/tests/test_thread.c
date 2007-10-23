#include <scelib/thread.h>
#include <stdarg.h>
#include <stdio.h>

lock_t printlock;

struct params
{
	char *name;
	int sleeptime;
};

void tprint(thread_t t, char *format, ...)
{
	va_list va;
	va_start(va, format);
	fflush(stdout);
	printf("[%p] ", (void *) t);
	vprintf(format, va);
	va_end(va);
	fflush(stdout);
}

void mprint(char *format, ...)
{
	va_list va;

	thread_lock(printlock);

	va_start(va, format);
	vprintf(format, va);
	va_end(va);
	fflush(stdout);

	thread_unlock(printlock);
}

void threadfunc(thread_t self, void *data)
{
	struct params *p = (struct params *) data;

	thread_lock(printlock);
	tprint(self, "entering %s\n", p->name);
	thread_unlock(printlock);

	thread_lock(printlock);
	tprint(self, "going to sleep %d seconds...\n", p->sleeptime);
	thread_unlock(printlock);
	thread_sleep(p->sleeptime);

	thread_lock(printlock);
	tprint(self, "leaving %s\n", p->name);
	thread_unlock(printlock);
	thread_exit(p->sleeptime);
}

int main(int argc, char **argv)
{
	thread_t t1, t2;
	struct params p1, p2;

	thread_lock_new(printlock);

	p1.name = "thread 1";
	p1.sleeptime = 3;

	p2.name = "thread 2";
	p2.sleeptime = 4;

	if ((t1 = thread_new(threadfunc, &p1)) == NULL)
		return 1;
	mprint("created thread 1\n");

	if ((t2 = thread_new(threadfunc, &p2)) == NULL)
		return 1;
	mprint("created thread 2\n");

	thread_start(t2);
	mprint("started thread 2\n");

	thread_start(t1);
	mprint("started thread 1\n");

	mprint("thread 1 returned %d\n", thread_waitfor(t1));
	mprint("thread 2 returned %d\n", thread_waitfor(t2));

	thread_lock_delete(printlock);

	return 0;
}
