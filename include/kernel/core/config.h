#ifndef _WAVECAT_CONFIG_H_
#define _WAVECAT_CONFIG_H_

#include <assert.h>
#include <stdio.h>
#include <math.h>

#include <kernel/core/cgi.h>

#define WAVECAT_ERROR(err) \
	fprintf(stderr, "Error: %d (%s:%d:%s)\n", (err), __FILE__, __LINE__, \
			__FUNCTION__)

#define WAVECAT_DEBUG_POINT() \
	fprintf(stderr, "Debug point (%s:%d:%s)\n", __FILE__, __LINE__, \
			__FUNCTION__)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define CONFIG_CAT_MAX_PARAMETERS 32
#define CONFIG_CAT_MAX_VARIABLES  32
#define CONFIG_CAT_MAX_EQUATIONS  32
#define CONFIG_CAT_MAX_STORAGE    32

#define CONFIG_CACHE_MAX_ALLOC    (200 * 1024 * 1024)

/* Define the macro to perform parallel computation */
//#define CONFIG_PARALLEL_COMP
/* Define the macro to perform profiling */
#define CONFIG_PROFILING
/* Define the macro to perform result caching */
#define CONFIG_CACHE_RESULT

#if defined(CONFIG_CACHE_RESULT) && defined(CONFIG_PARALLEL_COMP)
#undef CONFIG_PARALLEL_COMP
#endif

#endif
