#ifndef _WAVECAT_CMPLX_EQUATION_H_
#define _WAVECAT_CMPLX_EQUATION_H_

#include <kernel/core/config.h>

#include <stdlib.h>
#include <memory.h>
#include <complex.h>

struct catastrophe_s;

typedef void (*cmplx_equation_function_t)(const struct catastrophe_s *const catastrophe,
		const double t, const complex double *y, complex double *const f);

struct cmplx_equation_s {
	cmplx_equation_function_t function;
	unsigned int num_equations;

	/* Initial values and results of computing are complex values */
	double complex initial_vector[CONFIG_CAT_MAX_EQUATIONS];
	double complex resulting_vector[CONFIG_CAT_MAX_EQUATIONS];

	double complex storage[CONFIG_CAT_MAX_STORAGE];
};

typedef struct cmplx_equation_s cmplx_equation_t;

static inline cmplx_equation_t *
construct_cmplx_equation(void)
{
	cmplx_equation_t *e;

	e = malloc(sizeof(*e));
	if (!e)
		return NULL;
	memset(e, 0, sizeof(*e));
	return e;
}

static inline void
destruct_cmplx_equation(cmplx_equation_t *e)
{
	if (!e)
		return;
	free(e);
}

#define cmplx_equation_set_function(equ, func) (equ)->function = (func)

#endif
