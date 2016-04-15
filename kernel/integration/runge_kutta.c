/**
 * kernel/integration/runge_kutta.c - implementation of the classical Runge-Kutta
 * method to integrate systems of ordinary differential equations (ODE).
 *
 * NOTES:
 *
 * An ddditional memory are allocated on the call stack because of performance
 * reasons only. It can be changed in the future.
 *
 * TODO:
 *
 * Possible performance fixes:
 *  - get rid of additional copying before and after the calculation.
 */

#include <kernel/integration/runge_kutta.h>

/**
 * runge_kutta() - simplest possible numeric method for non-complex systems
 * @start       start value of the t variable
 * @end         second value of the t variable
 * @step        step
 * @catastrophe pointer to a catastrophe descriptor
 */
void runge_kutta(const double start, const double end, const double step,
		catastrophe_t *const cat)
{
	equation_t *equation;

	double k1[CONFIG_CAT_MAX_EQUATIONS];
	double k2[CONFIG_CAT_MAX_EQUATIONS];
	double k3[CONFIG_CAT_MAX_EQUATIONS];
	double k4[CONFIG_CAT_MAX_EQUATIONS];
	double  y[CONFIG_CAT_MAX_EQUATIONS];
	double y1[CONFIG_CAT_MAX_EQUATIONS];

	double t, cur_t;
	unsigned int i;

	equation = cat->equation;

	for (i = 0; i < equation->num_equations; i++) {
		y[i] = equation->initial_vector[i];
	}

	t = start;

	while (t < end - step) {
		cur_t = t;
		equation->function(cat, cur_t, y, k1);
		for (i = 0; i < equation->num_equations; i++) {
			k1[i] = k1[i] * step;
			y1[i] = y[i] + k1[i] / 2.0;
		}
		cur_t = t + step / 2.0;
		equation->function(cat, cur_t, y1, k2);
		for (i = 0; i < equation->num_equations; i++) {
			k2[i] = k2[i] * step;
			y1[i] = y[i] + k2[i] / 2.0;
		}
		cur_t = t + step / 2.0;
		equation->function(cat, cur_t, y1, k3);
		for (i = 0; i < equation->num_equations; i++) {
			k3[i] = k3[i] * step;
			y1[i] = y[i] + k3[i];
		}
		cur_t = t + step;
		equation->function(cat, cur_t, y1, k4);
		for (i = 0; i < equation->num_equations; i++) {
			k4[i] = k4[i] * step;
			y[i] = y[i] +
				(k1[i] + 2.0 * k2[i] + 2.0 * k3[i] + k4[i]) /
				6.0;
		}

		t += step;
	}

	for (i = 0; i < equation->num_equations; i++) {
		equation->resulting_vector[i] = y[i];
	}
}
