#ifndef _WAVECAT_EQUATION_H_
#define _WAVECAT_EQUATION_H_

#include <wavecat/config.h>

#include <stdlib.h>
#include <memory.h>

struct catastrophe_s;

typedef void (*equation_function_t)(const struct catastrophe_s *const catastrophe,
		const double t, const double *y, double *const f);

struct equation_s {
	equation_function_t function;
	unsigned int num_equations;

	double initial_vector[CONFIG_CAT_MAX_EQUATIONS];
	double resulting_vector[CONFIG_CAT_MAX_EQUATIONS];

	double storage[CONFIG_CAT_MAX_STORAGE];
};

typedef struct equation_s equation_t;

static inline equation_t *
construct_equation(void)
{
	equation_t *e;

	e = malloc(sizeof(*e));
	if (!e)
		return NULL;
	memset(e, 0, sizeof(*e));
	return e;
}

static inline void
destruct_equation(equation_t *e)
{
	if (!e)
		return;
	free(e);
}

#define equation_set_function(equ, func) (equ)->function = (func)

#endif
