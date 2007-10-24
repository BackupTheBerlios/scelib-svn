#include "scelib/map.h"
#include "scelib/memory.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>


/* ========================================================================= */
/* internal types                                                            */

/* ------------------------------------------------------------------------- */
/* one list in the map array, several items in the list if two elements of   */
/* the map are mapped to the same hash value                                 */
typedef struct bucket_type bucket_t;
struct bucket_type
{
	void *key;
	void *data;
	bucket_t *next;
};

/* ------------------------------------------------------------------------- */
/* used as map_t                                                             */
typedef struct map_type
{
	bucket_t **buckets;
	int size;
	int count;
	map_hash_t hash;
	map_comp_t comp;
	map_alloc_t alloc;
	map_free_t free;
} map_type;

/* ------------------------------------------------------------------------- */
/* used as map_iter_t                                                        */
typedef struct map_iter_type
{
	map_t map;
	int index;
	bucket_t *bucket;
} map_iter_type;



/* ========================================================================= */
/* static internal variables                                                 */

/* Increasing sequence of valid (i.e. prime) table sizes to choose from. */
static const int table_sizes[] =
{
	11, 23, 47, 101, 199, 401, 797, 1601, 3203, 6397, 12799, 25601,
	51199, 102397, 204803, 409597, 819187, 1638431, 3276799, 6553621,
	13107197, 26214401
};

/* get number of items in the table */
static const int num_table_sizes = sizeof(table_sizes) / sizeof(table_sizes[0]);

/* average bucket length threshold that must be reached before a map grows */
static const double table_resize_factor = 2.0;



/* ========================================================================= */
/* static functions definitions                                              */

/* ------------------------------------------------------------------------- */
/* Allocates an array of bucket pointers and initialize all pointers to 0.   */
static bucket_t **buckets_alloc(int size)
{
	bucket_t **bs;

	if ((bs = (bucket_t **) malloc(size * sizeof(bucket_t *))) == NULL)
		return NULL;

	return memset(bs, 0, size * sizeof(bucket_t *));
}

/* ------------------------------------------------------------------------- */
/* Frees the bucket array and all its content (lists of buckets).            */
static void buckets_free(map_t map)
{
	int i;
	bucket_t *b, *n;

	for (i = 0; i < map->size; ++i)
	{
		b = map->buckets[i];
		while (b)
		{
			n = b->next;
			if (map->free)
				map->free(b->key);
			free(b);
			b = n;
		}
	}
	free(map->buckets);
}

/* ------------------------------------------------------------------------- */
/* Creates a new bucket and associate it the key and data.                   */
static bucket_t *bucket_new(map_t map, void *key, void *data)
{
	bucket_t *b;

	if ((b = (bucket_t *) malloc(sizeof(bucket_t))) == NULL)
		return NULL;

	if (map->alloc)
		b->key = map->alloc(key);
	else
		b->key = key;
	b->data = data;
	b->next = NULL;
	return b;
}

static void bucket_free(map_t map, bucket_t *b)
{
	if (map->free)
		map->free(b->key);
	free(b);
}

static bucket_t *bucket_find(map_t map, void *key)
{
	int hashval;
	bucket_t *b;

	errno = 0;

	if (map == NULL || key == NULL)
		return RETERROR(EINVAL, NULL);

	if ((hashval = map->hash(map->size, key)) >= map->size)
		return RETERROR(EINVAL, NULL);

	b = map->buckets[hashval];
	while (b)
	{
		if (!map->comp(b->key, key))
			return b;
	}
	return NULL;
}

/* ------------------------------------------------------------------------- */
/* Makes a primed size computation for efficiency.                           */
static int map_calc_size(int size)
{
	int i;

	if (size == MAP_SIZE_AUTO)
		size = table_sizes[0];

	/* ensure using only prime numbers */
	for (i = 0; i < num_table_sizes; ++i)
	{
		if (table_sizes[i] >= size)
		{
			size = table_sizes[i];
			break;
		}
	}

	if (i == num_table_sizes)
		return RETERROR(EINVAL, -1);
	return size;
}

/* ------------------------------------------------------------------------- */
/* Resizes the buckets array to the next prime size.                         */
static int map_resize(map_t map)
{
	int size, i;
	bucket_t *b;
	map_t newmap;

	for (i = 1; i < num_table_sizes; ++i)
	{
		if (table_sizes[i] > map->size)
		{
			size = table_sizes[i];
			break;
		}
	}
	if (i == num_table_sizes)
		return RETERROR(EINVAL, -1);

	newmap = map_new(size, map->hash, map->comp, map->alloc, map->free);
	for (i = 0; i < map->size; ++i)
	{
		b = map->buckets[i];
		while (b)
		{
			map_set(newmap, b->key, b->data, NULL);
			b = b->next;
		}
	}

	buckets_free(map);
	map->buckets = newmap->buckets;
	map->size = newmap->size;
	free(newmap);

	return 0;
}



/* ========================================================================= */
/* public functions                                                          */

/* ------------------------------------------------------------------------- */
int map_ptr_hash(int size, void *key)
{
	const unsigned char *k = key;
	int h = 0;

	while (*k)
		h *= 31, h += *k++;

	return h % size;
}

/* ------------------------------------------------------------------------- */
map_t map_new(int size, map_hash_t hash_func, map_comp_t comp_func,
			  map_alloc_t alloc_func, map_free_t free_func)
{
	map_t map;

	if ((size = map_calc_size(size)) == -1)
		return NULL;

	if ((map = (map_t) malloc(sizeof(map_type))) == NULL)
		return NULL;

	map->size = size;
	map->count = 0;
	map->hash = hash_func;
	map->comp = comp_func;
	map->alloc = alloc_func;
	map->free = free_func;

	if ((map->buckets = buckets_alloc(map->size)) == NULL)
	{
		SAFEERRNO(free(map));
		return NULL;
	}
	return map;
}

/* ------------------------------------------------------------------------- */
int map_count(map_t map)
{
	if (map == NULL)
		return RETERROR(EINVAL, -1);
	return map->count;
}

/* ------------------------------------------------------------------------- */
int map_clear(map_t map, int newsize)
{
	if (map == NULL)
		return RETERROR(EINVAL, -1);

	buckets_free(map);

	if ((map->size = map_calc_size(newsize)) < 0)
		return map->size;

	if ((map->buckets = buckets_alloc(map->size)) == NULL)
		return -1;

	return (map->count = 0);
}

/* ------------------------------------------------------------------------- */
int map_delete(map_t map)
{
	if (map == NULL)
		return RETERROR(EINVAL, -1);

	buckets_free(map);
	free(map);
	return 0;
}

/* ------------------------------------------------------------------------- */
void *map_find(map_t map, void *key)
{
	bucket_t *b = bucket_find(map, key);
	return (b ? b->key : NULL);
}

/* ------------------------------------------------------------------------- */
void *map_get(map_t map, void *key)
{
	bucket_t *b = bucket_find(map, key);
	return (b ? b->data : NULL);
}

/* ------------------------------------------------------------------------- */
int map_set(map_t map, void *key, void *data, void **olddata)
{
	int hashval, ret;
	int replace = 1;
	bucket_t *b, *p;

	if (map == NULL || key == NULL)
		return RETERROR(EINVAL, -1);

	if ((double) map->count / (double) map->size >= table_resize_factor)
	{
		if ((ret = map_resize(map)))
			return ret;
	}

	if ((hashval = map->hash(map->size, key)) >= map->size)
		return RETERROR(EINVAL, -1);

	b = map->buckets[hashval];
	while (b)
	{
		if (!map->comp(b->key, key))
		{
			mem_init(olddata, b->data);
			b->data = data;
			break;
		}
		p = b;
		b = b->next;
	}
	if (b == NULL)
	{
		bucket_t *newb = bucket_new(map, key, data);
		if (b == map->buckets[hashval])
			map->buckets[hashval] = newb;
		else
			p->next = newb;
	}
	return ++ map->count;
}

/* ------------------------------------------------------------------------- */
int map_unset(map_t map, void *key, void **olddata)
{
	int hashval;
	bucket_t *b, *p;

	if (map == NULL || key == NULL)
		return RETERROR(EINVAL, -1);

	if ((hashval = map->hash(map->size, key)) >= map->size)
		return RETERROR(EINVAL, -1);

	b = map->buckets[hashval];
	p = NULL;
	while (b)
	{
		if (!map->comp(b->key, key))
		{
			if (p)
				p->next = b->next;
			p = b->next;
			mem_init(olddata, b->data);
			bucket_free(map, b);
			b = p;
		}
	}
	return -- map->count;
}

/* ------------------------------------------------------------------------- */
map_iter_t map_iter_new(map_t map)
{
	map_iter_t iter = (map_iter_t) malloc(sizeof(map_iter_type));
	if (iter == NULL)
		return NULL;

	iter->map = map;
	iter->bucket = NULL;
	iter->index = -1;
	return iter;
}

/* ------------------------------------------------------------------------- */
void map_iter_delete(map_iter_t iter)
{
	free(iter);
}

/* ------------------------------------------------------------------------- */
int map_iter_next(map_iter_t iter, void **key, void **data)
{
	bucket_t *b;
	int i;

	if (iter->index == -1)
	{
		/* first iteration */
		for (i = 0; i < iter->map->size; ++i)
		{
			if (iter->map->buckets[i] != NULL)
			{
				b = iter->map->buckets[i];
				break;
			}
		}
		if (i == iter->map->size)
			return -1;
		iter->index = i;
	}
	else
	{
		b = iter->bucket->next;
		if (b == NULL)
		{
			for (i = iter->index + 1; i < iter->map->size; ++i)
			{
				if (iter->map->buckets[i] != NULL)
				{
					b = iter->map->buckets[i];
					break;
				}
			}
			if (i == iter->map->size)
				return -1;
			iter->index = i;
		}
	}
	iter->bucket = b;
	*key = b->key;
	*data = b->data;
	return 0;
}
