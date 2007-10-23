/*	scelib - Simple C Extension Library
 *  Copyright (C) 2005-2007 Richard 'riri' GILL <richard@houbathecat.info>
 *
 *  thread.c - multithreading handling functions.
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

#include "scelib/thread.h"
#include "scelib/platform.h"
#if PLATFORM_IS(UNIX)
#include <sys/select.h>
#include <sys/time.h>
#endif
#include <stdlib.h>
#include <errno.h>
#include <math.h>



/* ========================================================================= */
/* internal types                                                            */


/* ------------------------------------------------------------------------- */
/* used as lock_t                                                            */

typedef struct lock_type
{
#if PLATFORM_IS(UNIX)
	pthread_mutex_t handle;
#else
	CRITICAL_SECTION handle;
#endif
} lock_type;

/* ------------------------------------------------------------------------- */
/* used as thread_t                                                          */
typedef struct thread_type
{
#if PLATFORM_IS(UNIX)
	pthread_t handle;
	lock_t starter;
#else
	HANDLE handle;
#endif
} thread_type;

/* ------------------------------------------------------------------------- */
/* internal thread routine parameter for wrapper                             */
typedef struct thread_params_type
{
	thread_t self;
	thread_proc_t proc;
	void *arg;
} thread_params_t;



/* ========================================================================= */
/* static internal variables                                                 */

#if PLATFORM_IS(UNIX)
#define KEY_NULL	((pthread_key_t) 0)
static pthread_key_t s_key = KEY_NULL;
#else
#define KEY_NULL	0xFFFFFFFF
static DWORD s_key = KEY_NULL;
#endif



/* ========================================================================= */
/* static functions definitions                                              */

#if PLATFORM_IS(UNIX)
static void *thread_real_proc(void *params)
#else
static unsigned long __stdcall thread_real_proc(void *params)
#endif
{
	thread_t self = ((thread_params_t *) params)->self;
	thread_proc_t proc = ((thread_params_t *) params)->proc;
	void *arg = ((thread_params_t *) params)->arg;

#if PLATFORM_IS(UNIX)
	thread_lock(self->starter);
	thread_unlock(self->starter);
	thread_lock_delete(self->starter);
#endif
	free(params);

	proc(self, arg);

	/* should never go here */
#if PLATFORM_IS(UNIX)
	return (void *) 0;
#else
	return 0;
#endif
}



/* ========================================================================= */
/* public functions                                                          */

/* ------------------------------------------------------------------------- */
void thread_lock_new(lock_t lock)
{
	if (lock == NULL)
		return;

#if PLATFORM_IS(UNIX)
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
#if defined(PTHREAD_MUTEX_RECURSIVE_NP)
	pthread_mutexattr_settype_np(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
#else
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
#endif
	pthread_mutex_init(&lock->handle, &attr);
	pthread_mutexattr_destroy(&attr);
#else	/* !UNIX */
	InitializeCriticalSection(&lock->handle);
#endif
}

/* ------------------------------------------------------------------------- */
void thread_lock_delete(lock_t lock)
{
	if (lock == NULL)
		return;

#if PLATFORM_IS(UNIX)
	pthread_mutex_destroy(&lock->handle);
#else
	DeleteCriticalSection(&lock->handle);
#endif
}

/* ------------------------------------------------------------------------- */
void thread_lock(lock_t lock)
{
	int err = errno;
	if (lock == NULL)
		return;

#if PLATFORM_IS(UNIX)
	pthread_mutex_lock(&lock->handle);
#else
	EnterCriticalSection(&lock->handle);
#endif
	errno = err;
}

/* ------------------------------------------------------------------------- */
int thread_trylock(lock_t lock)
{
	int retval;
	int err = errno;
	if (lock == NULL)
		return 0;

#if PLATFORM_IS(UNIX)
	retval = ((pthread_mutex_trylock(&lock->handle) == 0) ? 0 : -1);
#else
	retval = ((TryEnterCriticalSection(&lock->handle) == TRUE) ? 0 : -1);
#endif
	errno = err;
	return retval;
}

/* ------------------------------------------------------------------------- */
void thread_unlock(lock_t lock)
{
	int err = errno;
	if (lock == NULL)
		return;

#if PLATFORM_IS(UNIX)
	pthread_mutex_unlock(&lock->handle);
#else
	LeaveCriticalSection(&lock->handle);
#endif
	errno = err;
}

/* ------------------------------------------------------------------------- */
void thread_data_set(void *data)
{
	int err = errno;

#if PLATFORM_IS(UNIX)
	if (s_key == KEY_NULL)
		pthread_key_create(&s_key, NULL);
	if (s_key != KEY_NULL)
		pthread_setspecific(s_key, data);
#else
	if (s_key == KEY_NULL)
		s_key = TlsAlloc();
	if (s_key != KEY_NULL)
		TlsSetValue(s_key, data);
#endif
	errno = err;
}

/* ------------------------------------------------------------------------- */
void *thread_data_get(void)
{
	int err = errno;
	void *retval;

#if PLATFORM_IS(UNIX)
	retval = pthread_getspecific(s_key);
#else
	retval = TlsGetValue(s_key);
#endif
	errno = err;
	return retval;
}

/* ------------------------------------------------------------------------- */
thread_t thread_new(thread_proc_t proc, void *arg)
{
	thread_t t = NULL;
	thread_params_t *params;
#if PLATFORM_IS(WINDOWS)
	unsigned long id;
#endif

	if (proc == NULL)
		return RETERROR(EINVAL, NULL);

	if ((t = (thread_t) malloc(sizeof(thread_type))) == NULL)
		return NULL;

	params = (thread_params_t *) malloc(sizeof(thread_params_t));
	if (params == NULL) {
		free(t);
		return RETERROR(ENOMEM, NULL);
	}

	params->self = t;
	params->proc = proc;
	params->arg = arg;

#if PLATFORM_IS(UNIX)
	thread_lock_new(t->starter);
	thread_lock(t->starter);
	if (pthread_create(&t->handle, NULL, thread_real_proc, params))
	{
		int err = errno;
		free(t);
		errno = err;
		return NULL;
	}
#else	/* !UNIX */
	t->handle = CreateThread(NULL, 0, thread_real_proc, params, CREATE_SUSPENDED, &id);
	if (t->handle == NULL)
	{
		DWORD err = GetLastError();
		free(t);
		SetLastError(err);
		return NULL;
	}
#endif
	return t;
}

/* ------------------------------------------------------------------------- */
void thread_start(thread_t t)
{
#if PLATFORM_IS(UNIX)
	thread_unlock(t->starter);
#else
	ResumeThread(t->handle);
#endif
}

/* ------------------------------------------------------------------------- */
int thread_waitfor(thread_t t)
{
	int retval;

#if PLATFORM_IS(UNIX)
	pthread_join(t->handle, (void **) &retval);
#else
	WaitForSingleObject(t->handle, INFINITE);
	GetExitCodeThread(t->handle, (LPDWORD) &retval);
#endif
	free(t);
	return retval;
}

/* ------------------------------------------------------------------------- */
void thread_detach(thread_t t)
{
#if PLATFORM_IS(UNIX)
	pthread_detach(t->handle);
#else
	CloseHandle(t->handle);
#endif
	SAFEERRNO(free(t));
}

/* ------------------------------------------------------------------------- */
void thread_yield(void)
{
#if PLATFORM_IS(UNIX)
	sched_yield();
#else
	Sleep(0);
#endif
}

/* ------------------------------------------------------------------------- */
void thread_sleep(double seconds)
{
	double s;
	int ms, err = errno;
#if PLATFORM_IS(UNIX)
	struct timeval tv;
#endif

	/* .xxx * 1000 => miliseconds */
	ms = (int) (modf(seconds, &s) * 1000);
#if PLATFORM_IS(UNIX)
	tv.tv_sec = (long) s;
	tv.tv_usec = ms * 1000;	/* microseconds */
	select(0, NULL, NULL, NULL, &tv);
#else
	Sleep((DWORD) (s * 1000) + ms);	/* miliseconds */
#endif
	errno = err;
}

/* ------------------------------------------------------------------------- */
void thread_exit(int retval)
{
#if PLATFORM_IS(UNIX)
	pthread_exit((void *) retval);
#else
	ExitThread((DWORD) retval);
#endif
}

/* vi:set ts=4 sw=4: */
