/*	scelib - Simple C Extension Library
 *  Copyright (C) 2005-2007 Richard 'riri' GILL <richard@houbathecat.info>
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
/** @file
 *	@brief Basic multi-threading support.
 *
 *	This module introduce basic multi-threading features to programs; basic
 *	because it's not a complete support, but a simple set of types and
 *	functions to let programs deal with concurrent programming. Sometimes, you
 *	need to simply create a parallel thread of execution, and you just need a
 *	quick way to create it, start it, wait for its terminaison, and protect
 *	peace of code from concurrent access. The multi-threading module of scelib
 *	just provides this, no more.
 */

#ifndef __SCELIB_THREAD_H
#define __SCELIB_THREAD_H

#include "defs.h"

SCELIB_BEGIN_CDECL

/** Thread locking object.
 *
 *	This 's an opaque type which represents a locking variable. It's
 *	implementation is the quickest and simpliest one from each supported
 *	platform.
 */
typedef struct lock_type *lock_t;

/** Thread object.
 *
 *	When you @ref thread_new() "create a thread", you get back this opaque
 *	structure pointer, which in fact is a handle to the created thread. You'll
 *	need it to perform subsequent operations on threads, such as
 *	@ref thread_waitfor() "waiting for them".
 */
typedef struct thread_type *thread_t;

/** User defined thread routine.
 *
 *	This function pointer prototype is how your thread routine function must
 *	look. Note that it returns nothing (void), but you have a way to send back
 *	a return code to the parent (creator) thread with thread_exit().
 *
 *	@param[in] thread	the current thread object
 *	@param[in] userdata	user data from thread_new()
 *	@see thread_exit(), thread_new()
 */
typedef void (*thread_proc_t)(thread_t thread, void *userdata);

/** Creates a new locker.
 *
 *	A locking variable must be initialized before any use. Just pass it to this
 *	function to do so.
 */
void thread_lock_new(lock_t lock);

/** Delete a previously created locker (should not be locked).
 *
 *	When a locking variable isn't needed anymore, you should free system
 *	allocated resources for it. Just pass it to this function to give back the
 *	system a free slot for such object.
 *
 *	A locking variable must be unlocked to be deleted.
 */
void thread_lock_delete(lock_t lock);

/** Locks the scoped portion of code.
 *
 *	If the locking variable is already locked, this will wait for the other
 *	thread to unlock it. If several threads wait for the same locking variable,
 *	the one which will get a lock isn't defined.
 *
 *	Note that scelib lock variables are recursive, so you must call
 *	thread_unlock() for each call to this function to actually get the lock.
 */
void thread_lock(lock_t lock);

/** Tries to lock the scoped portion of code.
 *
 *	This function tries to get the lock, but unlike thread_lock(), it doesn't
 *	wait for the locking object to be unlocked.
 *
 *	@return 1 if done the job of @ref thread_lock(), or 0 if the object is
 *			already locked.
 */
int thread_trylock(lock_t lock);

/** Unlocks the scoped portion of code.
 *
 *	When you get the execution exclusivity by locking a lock, you must unlock
 *	it somewhere, to let other threads continuing their action. As scelib lock
 *	objects are recursives, the thread_lock() and thread_unlock() functions go
 *	in pair.
 */
void thread_unlock(lock_t lock);

/** Stores a data in a thread specific memory area.
 *
 *
 */
void thread_data_set(void *data);

/** Gets a previously stored data from a thread specific memory area.
 *
 *
 */
void *thread_data_get(void);

/** Creates a suspended thread with the given routine.
 *
 *	Call this function to create a new thread of execution. The thread is
 *	created but not started (created suspended). You must explicitly start it
 *	with a call to thread_start().
 *
 *	@param[in] proc	the thread routine
 *	@param[in] arg	user defined data forwarded to thread routine
 *	@return a new suspended thread object.
 */
thread_t thread_new(thread_proc_t proc, void *arg);

/** Starts the previously created thread.
 *
 *	Once a thread is created, you can start it with this function.
 */
void thread_start(thread_t t);

/** Waits for terminaison of the given thread.
 *
 *	@return the thread exit code if it called thread_exit(), 0 otherwise.
 */
int thread_waitfor(thread_t t);

/** Detaches the thread from the current given one.
 *
 *	By this call, the thread object isn't accessed anymore, and thus the
 *	current thread can't (and so don't have to) wait for its terminaison.
 */
void thread_detach(thread_t t);

/** Yields the remaining allocated timeslice, to give it to other threads.
 *
 *	Sometimes you know that when a peace of code is finished, you can wait
 *	another time slice. If your current thread is in this situation and some
 *	execution time remains from its slice, you can give other threads a chance
 *	to wake up.
 */
void thread_yield(void);

/** Makes the current thread sleeping during seconds.
 *
 *	As the number is a floating point value, it can be defined as little as
 *	miliseconds (not less for system limitations).
 */
void thread_sleep(double seconds);

/** Exit current thread with a specific exit code.
 *
 *	Thread routines doesn't return any value, but they can return one to the
 *	thread which created it, using this function. See @ref thread_waitfor().
 *
 *	@param[in] retval	the exit code returned by thread_waitfor()
 */
void thread_exit(int retval);

SCELIB_END_CDECL

#endif /* __SCELIB_THREAD_H */
/* vi:set ts=4 sw=4: */
