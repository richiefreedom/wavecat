#ifndef _LIB_INTEGRATION_RUNGE_KUTTA_H_
#define _LIB_INTEGRATION_RUNGE_KUTTA_H_

#include <wavecat/catastrophe.h>
#include <wavecat/equation.h>

void runge_kutta(const double start, const double end, const double step,
		catastrophe_t *const cat);

#endif /* _LIB_INTEGRATION_RUNGE_KUTTA_H_ */
