#ifndef _LIB_INTEGRATION_CMPLX_RUNGE_KUTTA_H_
#define _LIB_INTEGRATION_CMPLX_RUNGE_KUTTA_H_

#include <kernel/core/catastrophe.h>
#include <kernel/core/cmplx_equation.h>

#include <complex.h>

void cmplx_runge_kutta(const double start, const double end, const double step,
		catastrophe_t *const cat);

#endif /* _LIB_INTEGRATION_CMPLX_RUNGE_KUTTA_H_ */
