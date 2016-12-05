#ifndef __CACHE_SIMPLE_H__
#define __CACHE_SIMPLE_H__

#include <kernel/core/point_array.h>
#include <kernel/core/config.h>
#include <complex.h>

struct cached_result {
	double         parameter[CONFIG_CAT_MAX_PARAMETERS];
	unsigned int   num_parameters;
	point_t        point;
};

int simple_cache_save_result(void **root, struct cached_result *result);
struct cached_result *simple_cache_search_result(void **root,
		struct cached_result *result);
struct cached_result *simple_cache_cached_result_alloc(void);

#endif
