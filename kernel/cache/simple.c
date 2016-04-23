#include <kernel/core/config.h>
#include <kernel/cache/simple.h>
#include <stdio.h>
#include <stdlib.h>
#include <search.h>
#include <pthread.h>

static unsigned int allocated_bytes = 0;

static int compare(const void *l, const void *r)
{
	const struct cached_result *left = l, *right = r;
	unsigned int i;

	for (i = 0; i < left->num_parameters; i++) {
		if (left->parameter[i] != right->parameter[i]) {
			if (left->parameter[i] < right->parameter[i])
				return -1;
			else
				return 1;
		}
	}

	/* Check variables alse if the objects are still identical. */
	for (i = 0; i < left->num_variables; i++) {
		if (left->variable[i] != right->variable[i]) {
			if (left->variable[i] < right->variable[i])
				return -1;

			return 1;
		}
	}

	return 0;
}

int simple_cache_save_result(void **root, struct cached_result *result)
{
	void *val;

	val = tsearch(result, root, compare);
	if (!val) {
		perror("[error] incorrect return value from tsearch()");
		return -1;
	}

	/*
	 * Specially handle the situation when the cache already has such
	 * result.
	 */
	if (*((struct cached_result **)val) != result)
		free(result);

	return 0;
}

struct cached_result *simple_cache_search_result(void **root,
		struct cached_result *result)
{
	void *ret;

	ret = tfind(result, root, compare);
	if (!ret)
		return NULL;

	return *((struct cached_result **) ret);
}

struct cached_result *simple_cache_cached_result_alloc(void)
{
	struct cached_result *result;
	unsigned int checked_bytes;

	checked_bytes = __sync_fetch_and_add(&allocated_bytes, sizeof(*result));
	if (checked_bytes >= CONFIG_CACHE_MAX_ALLOC) {
		checked_bytes = __sync_fetch_and_sub(&allocated_bytes,
				sizeof(*result));
		return NULL;
	}

	result = malloc(sizeof(*result));
	if (!result) {
		checked_bytes = __sync_fetch_and_sub(&allocated_bytes,
				sizeof(*result));
		perror("[error] Cannot allocate cached result object");
		return NULL;
	}

	return result;
}
