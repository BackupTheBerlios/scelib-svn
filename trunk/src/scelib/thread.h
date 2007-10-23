/*	scelib - Simple C Extension Library
 *  Copyright (C) 2005-2007 Richard 'riri' GILL <richard@houbathecat.info>
 *
 *  thread.h - multithreading handling declarations.
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

#ifndef __SCELIB_THREAD_H
#define __SCELIB_THREAD_H

#include "defs.h"

SCELIB_BEGIN_CDECL

/* ========================================================================= */
/* types                                                                     */

/* ------------------------------------------------------------------------- */
/* thread locking object */
typedef struct lock_type *lock_t;

/* ------------------------------------------------------------------------- */
/* thread object */
typedef struct thread_type *thread_t;

/* ------------------------------------------------------------------------- */
/* user defined thread routine */
typedef void (*thread_proc_t)(thread_t, void *);

/* ========================================================================= */
/* functions                                                                 */

/* ------------------------------------------------------------------------- */
/* Creates a new locker.                                                     */
void thread_lock_new(lock_t lock);

/* ------------------------------------------------------------------------- */
/* Delete a previously created locker (should not be locked).                */
void thread_lock_delete(lock_t lock);

/* ------------------------------------------------------------------------- */
/* Locks the scoped portion of code.                                         */
void thread_lock(lock_t lock);

/* ------------------------------------------------------------------------- */
/* Tries to lock the scoped portion of code. Returns 1 if done the job of    */
/* thread_lock(), or 0 if the object is already locked.                      */
int thread_trylock(lock_t lock);

/* ------------------------------------------------------------------------- */
/* Unlocks the scoped portion of code.                                       */
void thread_unlock(lock_t lock);

/* ------------------------------------------------------------------------- */
/* Stores a data in a thread specific memory area.                           */
void thread_data_set(void *data);

/* ------------------------------------------------------------------------- */
/* Gets a previously stored data from a thread specific memory area.         */
void *thread_data_get(void);

/* ------------------------------------------------------------------------- */
/* Creates a suspended thread with the given routine, and passing it 'arg'.  */
thread_t thread_new(thread_proc_t proc, void *arg);

/* ------------------------------------------------------------------------- */
/* Starts the previously created thread.                                     */
void thread_start(thread_t t);

/* ------------------------------------------------------------------------- */
/* Waits for terminaison of the given thread. If the thread returned a value */
/* with thread_exit(), it's returned here, 0 is returned otherwise.          */
int thread_waitfor(thread_t t);

/* ------------------------------------------------------------------------- */
/* Detaches the current thread from the given one (not accessing its thread  */
/* object anymore, and thus not waiting for its terminaison).                */
void thread_detach(thread_t t);

/* ------------------------------------------------------------------------- */
/* Yields the remaining allocated timeslice, to give it to other threads.    */
void thread_yield(void);

/* ------------------------------------------------------------------------- */
/* Makes the current thread sleeping during the given number of seconds (can */
/* define as little as miliseconds).                                         */
void thread_sleep(double seconds);

/* ------------------------------------------------------------------------- */
/* Thread routines doesn't return any value, but they can return one to the  */
/* thread which created it, using this function. See thread_waitfor().       */
void thread_exit(int retval);

SCELIB_END_CDECL

#endif /* __SCELIB_THREAD_H */
/* vi:set ts=4 sw=4: */
