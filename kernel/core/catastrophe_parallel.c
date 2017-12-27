#include <kernel/core/catastrophe.h>
#include <kernel/core/catastrophe_parallel.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>

#define MAX_THREADS 8

catastrophe_parallel_func_t catastrophe_parallel_loop;

static void *parallel_loop_thread(void *param)
{
	long res;
	catastrophe_t *cat = param;

	fprintf(stderr, "[parallel] Computational thread started.\n");
	res = catastrophe_loop(cat);
	fprintf(stderr, "[parallel] Computation finished.\n");

	return (void *) res;
}

void catastrophe_prepare_params(catastrophe_t *catastrophe,
		int p1_idx, int p2_idx, int cores)
{
	unsigned int p1_steps, p2_steps;
	double p1_min, p1_max, p1_diff, p1_step;
	double p2_min, p2_max, p2_diff, p2_step;

	p1_min   = catastrophe->parameter[p1_idx].min_value;
	p1_max   = catastrophe->parameter[p1_idx].max_value;
	p1_steps = catastrophe->parameter[p1_idx].num_steps;
	p1_step  = (p1_max - p1_min) / p1_steps;

	catastrophe->parameter[p1_idx].step_size = p1_step;

	p2_min   = catastrophe->parameter[p2_idx].min_value;
	p2_max   = catastrophe->parameter[p2_idx].max_value;
	p2_steps = catastrophe->parameter[p2_idx].num_steps;
	p2_step  = (p2_max - p2_min) / p2_steps;

	catastrophe->parameter[p2_idx].step_size = p2_step;
}

int catastrophe_loop_seq(catastrophe_t *catastrophe)
{
	uint_pair_t pair;

	pair = catastrophe_search_alterable_params(catastrophe);
	if (!catastrophe_is_pair_correct(&pair)) {
		WAVECAT_ERROR(-1);
		return -1;
	}

	catastrophe_prepare_params(catastrophe, pair.first, pair.second, 1);

	return catastrophe_loop(catastrophe);
}

int catastrophe_loop_smp(catastrophe_t *catastrophe)
{
	int res, is_failed = 0;
	unsigned int p1_idx, cores, thread_idx, first_idx, steps_per_core;
	double p1_min, p1_max, p1_diff, p1_step;

	catastrophe_desc_t *catastrophe_desc;
	catastrophe_t *new_cat;
	uint_pair_t pair;

	catastrophe_t *tcatastrophe[MAX_THREADS + 1];
	pthread_t      thread[MAX_THREADS + 1];

	cores = 4; /* TODO: get real number of cores */

	pair = catastrophe_search_alterable_params(catastrophe);
	if (!catastrophe_is_pair_correct(&pair)) {
		WAVECAT_ERROR(-1);
		return -1;
	}

	catastrophe_prepare_params(catastrophe, pair.first, pair.second, cores);

	p1_min   = catastrophe->parameter[pair.first].min_value;
	p1_max   = catastrophe->parameter[pair.first].max_value;
	p1_step  = catastrophe->parameter[pair.first].step_size;

	steps_per_core = catastrophe->parameter[pair.first].num_steps / cores;

	catastrophe_desc =
		find_catastrophe_desc(catastrophe->sym_name);
	if (!catastrophe_desc) {
		WAVECAT_ERROR(-ENOMEM);
		return -1;
	}

	thread_idx = cores;
	while (thread_idx--) {
		catastrophe->parameter[pair.first].min_value = p1_min;
		p1_max = p1_step * steps_per_core + p1_min;
		catastrophe->parameter[pair.first].max_value = p1_max;
		catastrophe->parameter[pair.first].num_steps = steps_per_core;
		p1_min = p1_max;

		new_cat = catastrophe_desc->fabric(catastrophe_desc,
				catastrophe->parameter, catastrophe->deriv);

		if (!new_cat) {
			WAVECAT_ERROR(-1);
			goto fail;
		}

		tcatastrophe[thread_idx] = new_cat;

		res = pthread_create(&thread[thread_idx], NULL,
				parallel_loop_thread,
				(void *) new_cat);
		if (res) {
			WAVECAT_ERROR(-1);
			goto fail;
		}
	}

	thread_idx = cores;
	first_idx = 0;
	while (thread_idx--) {
		void *retval = NULL;

		res = pthread_join(thread[thread_idx], &retval);
		if (res) {
			WAVECAT_ERROR(-1);
			goto fail;
		}

		if (retval)
			is_failed = 1;

		copy_part_point_array(catastrophe->point_array,
				tcatastrophe[thread_idx]->point_array,
				first_idx);
		first_idx +=
			tcatastrophe[thread_idx]->point_array->num_steps_x;

		destruct_catastrophe(tcatastrophe[thread_idx]);
	}

	if (is_failed)
		goto fail;

	return 0;

fail:
	return -1;
}

static void catastrophe_parallel(void) __attribute__((constructor));
static void catastrophe_parallel(void)
{
#ifdef CONFIG_PARALLEL_COMP
	catastrophe_parallel_loop = catastrophe_loop_smp;
#else
	catastrophe_parallel_loop = catastrophe_loop_seq;
#endif
}
