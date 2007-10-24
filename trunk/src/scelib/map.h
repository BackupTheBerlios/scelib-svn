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
 *	@brief Map/Dictionnary handling.
 *
 */
#ifndef __SCELIB_MAP_H
#define __SCELIB_MAP_H

#include "defs.h"

SCELIB_BEGIN_CDECL

/** Tells the map object to compute by itself its size.
 *
 *	The map_t type uses internally a table to store data, and well defining its
 *	size is mandatory to acheive good performances. In most cases, you can let
 *	the map @ref map_new() "creation function" compute the table size by
 *	itself, passing it this define.
 */
#define MAP_SIZE_AUTO		-1

/** The map object.
 *
 *	The map object is an opaque structure, and you access it only by this
 *	handle type.
 */
typedef struct map_type *map_t;

/** Pointer to function computing a hash key.
 *
 *	The map lookup uses a hash value to choose the data store position, based
 *	on its table size. Each hash function must have this prototype. You pass
 *	such a function pointer to the map_new() function.
 *
 *	@param[in] size	the size of the map table
 *	@param[in] key	data key to compute the hash for
 *	@return the hash value.
 */
typedef int (*map_hash_t)(int size, void *key);

/** Pointer to function comparing two keys.
 *
 *	When the map searches for a key, it compares some of them in its store
 *	area, and uses this function prototype for that. You pass such a function
 *	pointer to the map_new() function.
 *
 *	@param[in] key1
 *	@param[in] key2
 *	@return 0 if keys are equals, -1 if first key is @e before the second one,
 *			and 1 if the first key is @e after the second one.
 */
typedef int (*map_comp_t)(void *key1, void *key2);

/** Pointer to function allocating key.
 *
 *	You may want to duplicate the key value when setting a new key/value pair
 *	in the map. To do so, the map will call a function based on this pointer
 *	type, if defined. You pass such a function pointer to the map_new()
 *	function.
 *
 *	@param[in] key	the key to duplicate
 *	@return a pointer to the duplicated key memory area.
 */
typedef void* (*map_alloc_t)(void *key);

/** Pointer to function deallocating key.
 *
 *	When the map duplicates keys with a map_alloc_t function, it needs to free
 *	them when it delete the key, or when the map object is destroyed itself.
 *	You pass such a function pointer to the map_new() function.
 *
 *	@param[in] key	pointer to the key data memory area to free
 */
typedef void (*map_free_t)(void *key);

/** Object to iterate in a map object.
 *
 *	This opaque type is a structured handle to an iteration object, permitting
 *	to traverse a map. This handle type is used in all map iteration functions.
 */
typedef struct map_iter_type *map_iter_t;

/** Default hash function.
 *
 *	Classic and efficient hash function, which works well with pointers, but
 *	also with strings (aka char pointers). You would never call this function
 *	directly, but pass its pointer to the map_new() function.
 *
 *	@see map_hash_t, map_new()
 */
int map_ptr_hash(int size, void *key);

/** Creates a new map object.
 *
 *	This function allocates all needed data to let you use a map/dictionnary,
 *	and gives you back a handle to this object.
 *
 *	@param[in] size			initial size of the map. Set it to MAP_SIZE_AUTO
 *							if you don't want to bother with the map table
 *							size
 *	@param[in] hash_func	hash function compatible with the map_hash_t
 *							prototype
 *	@param[in] comp_func	comparaison function compatible with the map_comp_t
 *							prototype
 *	@param[in] alloc_func	allocation function to duplicate the key memory.
 *							Use NULL if duplication isn't needed
 *	@param[in] free_func	deallocation function to free key memory duplicated
 *							with @a alloc_func
 *	@return a pointer to the newly created map object, or NULL if any error.
 *			Actual error can be obtained with errno.
 */
map_t map_new(int size, map_hash_t hash_func, map_comp_t comp_func,
			  map_alloc_t alloc_func, map_free_t free_func);

/** Returns the number of elements in the map.
 *
 *	This function is obvious :-)
 *
 *	@param[in] map	the map object
 *	@return the number of items actually in the map, or -1 if an invalid map
 *			object was specified.
 */
int map_count(map_t map);

/** Clears the content of the map object, giving its new size.
 *
 *	This function removes all key/value pairs stored in the map and recreate
 *	the internal table.
 *
 *	@param[in] map		the map object to clear
 *	@param[in] newsize	the new initial size of the map, which can be
 *						MAP_SIZE_AUTO
 *	@return 0 (no element) if ok, or -1 if an invalid map object was specified,
 *	if an allocation error occurred, or if the new size is too big (the map
 *	utility uses a table of prime numbers to calculate efficient space of
 *	items, and this table is limited).
 */
int map_clear(map_t map, int newsize);

/** Destroys the map object.
 *
 *	This frees the map object and all associated data. If key duplication
 *	occured during the map fill, the corresponding free will be performed
 *	on keys.
 *
 *	@param[in] map	the map to destroy
 *	@return -1 is an invalid map object was specified, or 0.
 */
int map_delete(map_t map);

/** Finds the real key memory area in the map.
 *
 *	When you tell the map to duplicate keys during fill, of course the key is
 *	internaly stored in a different memory area than your @a key. This function
 *	gives you a chance to get the internal memory pointer.
 *
 *	@param[in] map	the map object
 *	@param[in] key	the key to find
 *	@return a pointer to the internal key memory area, or NULL if any error or
 *			if the key wasn't found.
 */
void *map_find(map_t map, void *key);

/** Retrieves the data associated with the key.
 *
 *	This's the @e read part of the map utility. Provided a @a key, it gives the
 *	associated value.
 *
 *	@param[in] map	the map object
 *	@param[in] key	the key part of the key/value pair
 *	@return a pointer to the value or NULL is not found or an error occurred
 *			during the hash computation (see errno with EINVAL if such error).
 */
void *map_get(map_t map, void *key);

/** Associates the key with the given value.
 *
 *	This's the @e write part of the map utility. You gives a @a key and a
 *	@a value to store in the map. If the key is found in the map, the value
 *	is replaced. The map_alloc_t function passed to map_new() is called if
 *	defined.
 *
 *	@param[in] map		the map object
 *	@param[in] key		the key to create or modify
 *	@param[in] data		the new data
 *	@param[out] olddata	back pointer to the old data, or NULL (place NULL if
 *						you don't want to get the old value
 *	@return the new item count of the map object, or -1 if any error.
 */
int map_set(map_t map, void *key, void *data, void **olddata);

/** Delete the key/value pair from the map.
 *
 *	When you don't want a key/value pair to be stored in the map, you unset it.
 *	The map_free_t function passed to map_new() is called if defined.
 *
 *	@param[in] map		the map object
 *	@param[in] key		the key to find to delete the pair
 *	@param[out] olddata	back pointer to the old data, or NULL (place NULL if
 *						you don't want to get the old value
 *	@return the new item cound of the map object, or -1 if any error.
 */
int map_unset(map_t map, void *key, void **olddata);

/** Creates a new iteration object.
 *
 *	@see map_iter_t
 *	@param[in] map	the map object to associate the iterator with
 *	@return a new iterator object, or NULL if allocation error occurred (see
 *			errno).
 */
map_iter_t map_iter_new(map_t map);

/** Destroy a map iteration object (do not delete the map!).
 *
 *	When you don't need the iteration object anymore, you free it to avoid
 *	leaks.
 *
 *	@param[in] iter	the iterator object to delete
 */
void map_iter_delete(map_iter_t iter);

/** Get the next (or first) key/value pair from the map.
 *
 *	This's the iteration function, which permit to traverse the map. Note that
 *	data isn't sorted, so you won't get the key/value pair in the order you
 *	inserted them.
 *
 *	@param[in] iter		the iteration object
 *	@param[out] key		the next key in the map
 *	@param[out] data	the value associated with the key
 *	@return 0 if the key/value pair could be retrieved, -1 if reached the end
 *			of the map.
 */
int map_iter_next(map_iter_t iter, void **key, void **data);

SCELIB_END_CDECL

#endif /* __SCELIB_MAP_H */
/* vi:set ts=4 sw=4: */
