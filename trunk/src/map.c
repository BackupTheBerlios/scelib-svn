#include "scelib/map.h"
#include "scelib/memory.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _DEBUG
#include <stdio.h>
#define DPRINT(m)	printf m
#else
#define DPRINT(m)
#endif

typedef struct bucket_type
{
	void *key;
	void *data;
	struct bucket_type *next;
} bucket_t;

struct map_type
{
	bucket_t **buckets;
	int size;
	int count;
	map_hash_t hashf;
	map_comp_t compf;
	map_alloc_t allocf;
	map_free_t freef;
};

struct map_iter_type
{
	map_t map;
	bucket_t *bucket;
	int index;
	int count;
};

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

static bucket_t *map_bucket_find(map_t map, void *key)
{
	bucket_t *b;

	b = map->buckets[map->hashf(map->size, key)];
	while (b)
	{
		if (!map->compf(key, b->key))
		{
			DPRINT(("found bucket at %p for key %p\n", b, key));
			return b;
		}
		b = b->next;
	}
	return 0;
}

static bucket_t *map_bucket_alloc(map_t map, void *key, void *data)
{
	bucket_t *b;

	if (!(b = (bucket_t *) malloc(sizeof(bucket_t))))
		return 0;

	b->key = (map->allocf ? map->allocf(key) : key);
	if (!b->key)
	{
		SAFEERRNO(free(b));
		return 0;
	}
	DPRINT(("allocated bucket at %p\n", b));
	if (map->allocf)
		DPRINT(("copied key at %p\n", b->key));
	b->data = data;
	b->next = 0;
	return b;
}

static bucket_t *map_bucket_free(map_t map, bucket_t *bucket)
{
	bucket_t *next;
	if (map->freef)
	{
		DPRINT(("calling key free function for %p\n", bucket->key));
		map->freef(bucket->key);
	}
	next = bucket->next;
	DPRINT(("freeing bucket at %p\n", bucket));
	free(bucket);
	return next;
}

static void map_set_buckets(map_t map, bucket_t **buckets, int size)
{
	if (map->buckets)
	{
		int i;
		bucket_t *b;
		for (i = 0; i < map->size; ++i)
		{
			b = map->buckets[i];
			while (b)
				b = map_bucket_free(map, b);
		}
		DPRINT(("freeing buckets table at %p\n", map->buckets));
		free(map->buckets);
	}
	DPRINT(("settings buckets table to %p\n", buckets));
	map->buckets = buckets;
	map->size = size;
}

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
		return RETERROR(ERANGE, -1);
	DPRINT(("calculated table size to %d\n", size));
	return size;
}

static int map_resize(map_t map, int newsize, int force)
{
	if ((newsize = map_calc_size(newsize)) == -1)
		return -1;

	if (force || newsize > map->size)
	{
		map_t newmap;
		int i;
		bucket_t *b;

		newmap = map_new(newsize, map->hashf, map->compf, map->allocf, map->freef);
		if (!newmap)
			return -1;
		for (i = 0; i < map->size; ++i)
		{
			b = map->buckets[i];
			while (b)
			{
				map_set(newmap, b->key, b->data, 0);
				b = b->next;
			}
		}
		map_set_buckets(map, newmap->buckets, newsize);
		free(newmap);
	}
	return map->size;
}

static int map_iter_nextbucket(map_iter_t iter, int startindex)
{
	int i;

	for (i = startindex; i < iter->map->size; ++i)
	{
		if (iter->map->buckets[i])
		{
			iter->bucket = iter->map->buckets[i];
			return i;
		}
	}
	iter->bucket = 0;
	return -1;
}

int map_ptr_hash(int size, void *key)
{
	const unsigned char *k = key;
	unsigned int h = 0;

	while (*k)
		h *= 31, h += *k++;
	h %= size;
	DPRINT(("calculated hash %d for key %p\n", h, key));
	return h;
}

map_t map_new(int size, map_hash_t hash_func, map_comp_t comp_func,
			  map_alloc_t alloc_func, map_free_t free_func)
{
	map_t map;

	if ((size = map_calc_size(size)) == -1)
		return 0;

	if (!(map = (map_t) malloc(sizeof(struct map_type))))
		return NULL;

	if (!(map->buckets = (bucket_t **) calloc(size, sizeof(bucket_t *))))
	{
		SAFEERRNO(free(map));
		return 0;
	}
	map->size = size;
	map->count = 0;
	map->hashf = hash_func;
	map->compf = comp_func;
	map->allocf = alloc_func;
	map->freef = free_func;

	DPRINT(("allocated map at %p\n", map));
	DPRINT(("allocated buckets table at %p\n", map->buckets));
	return map;
}

int map_count(map_t map)
{
	if (!map)
		return RETERROR(EINVAL, -1);
	return map->count;
}

int map_clear(map_t map, int newsize)
{
	int i;
	bucket_t *b;

	if (!map || !newsize)
		return RETERROR(EINVAL, -1);

	for (i = 0; i < map->size; ++i)
	{
		b = map->buckets[i];
		while (b)
			b = map_bucket_free(map, b);
		map->buckets[i] = 0;
	}
	DPRINT(("clear buckets table at %p\n", map->buckets));
	map->count = 0;

	return (map_resize(map, newsize, 1) > 0 ? 0 : -1);
}

int map_delete(map_t map)
{
	if (!map)
		return RETERROR(EINVAL, -1);

	map_set_buckets(map, 0, 0);
	DPRINT(("freeing map at %p\n", map));
	free(map);
	return 0;
}

void *map_find(map_t map, void *key)
{
	bucket_t *b;

	if (!map || !key)
		return RETERROR(EINVAL, 0);

	b = map_bucket_find(map, key);
	return (b ? b->key : 0);
}

void *map_get(map_t map, void *key)
{
	bucket_t *b;

	if (!map || !key)
		return RETERROR(EINVAL, 0);

	b = map_bucket_find(map, key);
	return (b ? b->data : 0);
}

int map_set(map_t map, void *key, void *data, void **olddata)
{
	bucket_t *b;

	if (!map || !key)
		return RETERROR(EINVAL, -1);

	b = map_bucket_find(map, key);
	if (b)
	{
		mem_init(olddata, b->data);
		b->data = data;
	}
	else
	{
		int hash;

		if (!map_resize(map, map->count + 1, 0))
			return -1;

		if (!(b = map_bucket_alloc(map, key, data)))
			return -1;

		hash = map->hashf(map->size, key);
		b->next = map->buckets[hash];
		map->buckets[hash] = b;
		++ map->count;
	}

	return map->count;
}

int map_unset(map_t map, void *key, void **olddata)
{
	bucket_t *b, *prev;
	int hash;

	if (!map || !key)
		return RETERROR(EINVAL, -1);

	hash = map->hashf(map->size, key);
	b = map->buckets[hash];
	prev = 0;
	while (b)
	{
		if (!map->compf(key, b->key))
		{
			if (prev)
				prev->next = b->next;
			else
				map->buckets[hash] = b->next;
			mem_init(olddata, b->data);
			map_bucket_free(map, b);
			return -- map->count;;
		}
		prev = b;
		b = b->next;
	}
	return RETERROR(ERANGE, -1);
}

map_iter_t map_iter_new(map_t map)
{
	map_iter_t iter;

	if (!map)
		return RETERROR(EINVAL, 0);

	if (!(iter = (map_iter_t) malloc(sizeof(struct map_iter_type))))
		return NULL;

	iter->map = map;
	iter->bucket = NULL;
	iter->index = -1;
	iter->count = 0;

	DPRINT(("iterator allocated at %p\n", iter));
	return iter;
}

int map_iter_delete(map_iter_t iter)
{
	if (!iter)
		return RETERROR(EINVAL, -1);

	free(iter);
	return 0;
}

int map_iter_next(map_iter_t iter, void **key, void **data)
{
	if (!iter)
		return RETERROR(EINVAL, -1);

	if (iter->index == -1)
	{
		iter->index = map_iter_nextbucket(iter, 0);
	}
	else
	{
		bucket_t *b = iter->bucket->next;
		if (!b)
			iter->index = map_iter_nextbucket(iter, iter->index + 1);
		else
			iter->bucket = b;
	}
	if (!iter->bucket)
		return 0;

	mem_init(key, iter->bucket->key);
	mem_init(data, iter->bucket->data);

	return ++ iter->count;
}

#ifdef _DEBUG
int map_dump(map_t map)
{
	bucket_t *b;
	int i;

	if (!map)
		return RETERROR(EINVAL, -1);

	printf("map at #%p (size %d, %d elements)\n", map, map->size, map->count);
	printf("buckets table at #%p\n", map->buckets);
	for (i = 0; i < map->size; ++i)
	{
		b = map->buckets[i];
		if (!b)
		{
			printf("[%d]: empty\n", i);
			continue;
		}
		printf("[%d]: #%p - key %p - data %p\n", i, b, b->key, b->data);
		b = b->next;
		while (b)
		{
			printf("   -> #%p - key %p - data %p\n", b, b->key, b->data);
			b = b->next;
		}
	}
	return 0;
}

#endif
