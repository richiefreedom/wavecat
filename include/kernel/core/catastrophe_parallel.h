#ifndef WAVECAT_CATASTROPHE_PARALLEL_H
#define WAVECAT_CATASTROPHE_PARALLEL_H

typedef int (*catastrophe_parallel_func_t)(catastrophe_t *catastrophe);
extern catastrophe_parallel_func_t catastrophe_parallel_loop;

#endif /* WAVECAT_CATASTROPHE_PARALLEL_H */
