#include <kernel/core/catastrophe.h>
#include <kernel/core/equation.h>
#include <kernel/core/cmplx_equation.h>
#include <kernel/cache/simple.h>
#include <kernel/core/config.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

static DECLARE_LIST_HEAD(catastrophe_desc_list);

void register_catastrophe_desc(catastrophe_desc_t *cd)
{
#ifdef CONFIG_CACHE_RESULT
#ifdef CONFIG_PARALLEL_COMP
	pthread_spin_init(&cd->cache_root_lock, PTHREAD_PROCESS_PRIVATE);
#endif
	cd->cache_root = NULL;
#endif
	list_add_tail(&cd->list, &catastrophe_desc_list);
}

void unregister_catastrophe_desc(catastrophe_desc_t *cd)
{
	list_head_t *pos;
	int found = 0;

	list_for_each(pos, &catastrophe_desc_list) {
		if (pos == &cd->list) {
			found++;
			break;
		}
	}

	if (found)
		list_del(pos);
}

catastrophe_desc_t *find_catastrophe_desc(const char *sym_name)
{
	catastrophe_desc_t *cd;
	list_head_t *pos;

	list_for_each(pos, &catastrophe_desc_list) {
		cd = list_entry(pos, catastrophe_desc_t, list);
		if (strcmp(cd->sym_name, sym_name) == 0)
			return cd;
	}

	return NULL;
}

int catastrophe_loop(catastrophe_t *const catastrophe)
{
	unsigned int i, j;
	uint_pair_t pair;

	fprintf(stderr, "Catastrophe %s calculation.\n",
			catastrophe->sym_name);

	pair = catastrophe_search_alterable_params(catastrophe);
	if (!catastrophe_is_pair_correct(&pair)) {
		WAVECAT_ERROR(-1);
		return -1;
	}

	unsigned int p1_idx = pair.first;
	unsigned int p2_idx = pair.second;

	double p1_min = catastrophe->parameter[p1_idx].min_value;
	double p2_min = catastrophe->parameter[p2_idx].min_value;
	double p1_max = catastrophe->parameter[p1_idx].max_value;
	double p2_max = catastrophe->parameter[p2_idx].max_value;
	double p1_steps = catastrophe->parameter[p1_idx].num_steps;
	double p2_steps = catastrophe->parameter[p2_idx].num_steps;
	double p1_step_size = catastrophe->parameter[p1_idx].step_size;
	double p2_step_size = catastrophe->parameter[p2_idx].step_size;

	point_array_t *pa = catastrophe->point_array;

#ifdef CONFIG_CACHE_RESULT
	struct cached_result  temp_key;
	struct cached_result *result;

	temp_key.num_parameters = catastrophe->num_parameters;
	for (i = 0; i < catastrophe->num_parameters; i++) {
		temp_key.parameter[i] = catastrophe->parameter[i].cur_value;
	}
#endif

	for (i = 0; i < p1_steps; i++) {
		/* Calculate the current value of the parameter */
		catastrophe->parameter[p1_idx].cur_value =
			p1_min + i * p1_step_size;
#ifdef CONFIG_CACHE_RESULT
		temp_key.parameter[p1_idx] =
			catastrophe->parameter[p1_idx].cur_value;
#endif
		for (j = 0; j < p2_steps; j++) {
			catastrophe->parameter[p2_idx].cur_value =
				p2_min + j * p2_step_size;
#ifdef CONFIG_CACHE_RESULT
			temp_key.parameter[p2_idx] =
				catastrophe->parameter[p2_idx].cur_value;

#ifdef CONFIG_PARALLEL_COMP
			pthread_spin_lock(
				&catastrophe->descriptor->cache_root_lock);
#endif
			result = simple_cache_search_result(
					&catastrophe->descriptor->cache_root,
					&temp_key);
#ifdef CONFIG_PARALLEL_COMP
			pthread_spin_unlock(
				&catastrophe->descriptor->cache_root_lock);
#endif
			if (result) {
				pa->array[i][j].module =
					result->point.module;
				pa->array[i][j].phase =
					result->point.phase;
				continue;
			}
#endif
			catastrophe->calculate(catastrophe, i, j);

			/* 
			 * Check the result of calculation in the point.
			 * In the case of infinum value the calculations
			 * must be stopped.
			 */
			if (pa->array[i][j].module > 100 ||
				pa->array[i][j].module < -100) {
				WAVECAT_ERROR(-1);
				return -1;
			}

#ifdef CONFIG_CACHE_RESULT
#ifdef CONFIG_PARALLEL_COMP
			pthread_spin_lock(
				&catastrophe->descriptor->cache_root_lock);
#endif
			result = simple_cache_cached_result_alloc();
#ifdef CONFIG_PARALLEL_COMP
			pthread_spin_unlock(
				&catastrophe->descriptor->cache_root_lock);
#endif
			if (!result)
				continue;

			memcpy(result, &temp_key, sizeof(*result));
			result->point.module = pa->array[i][j].module;
			result->point.phase = pa->array[i][j].phase;
			simple_cache_save_result(
					&catastrophe->descriptor->cache_root,
					result);
#endif
		}
	}

	return 0;
}

static int bind_parameter_names(catastrophe_desc_t *desc,
		catastrophe_t *cat, parameter_t *par)
{
	unsigned int i, j;

	for (i = 0; desc->par_names[i] != NULL; i++) {
		assert(i < desc->num_parameters);
		for (j = 0; j < desc->num_parameters; j++) {
			if (strcmp(par[j].sym_name, desc->par_names[i]) == 0) {
				cat->parameter[i] = par[j];
				break;
			}
		}
		if (j == desc->num_parameters) {
			fprintf(stderr, "Error: unbound parameter %s.\n",
					desc->par_names[i]);
			CGI_ERROR("Not all parameters are bound");
			return -1;
		}
	}

	cat->num_parameters = desc->num_parameters;

	return 0;
}

catastrophe_t *catastrophe_fabric(catastrophe_desc_t *desc,
		parameter_t *parameter)
{
	catastrophe_t *catastrophe;
	point_array_t *point_array;
	void *equation;
	uint_pair_t pair;

	int err = 0;
	unsigned int i;

	catastrophe = construct_catastrophe();
	if (!catastrophe)
		goto error;

	catastrophe->descriptor = desc;

	if (desc->par_names) {
		if (bind_parameter_names(desc, catastrophe, parameter))
			goto error_bind_params;
	} else {
		for (i = 0; i < desc->num_parameters; i++)
			catastrophe->parameter[i] = parameter[i];
		catastrophe->num_parameters = desc->num_parameters;
	}

	switch (desc->type) {
	case CT_REAL: {
		equation_t *tequation;
		tequation = construct_equation();
		if (!tequation)
			goto error_construct_equation;
		equation_set_function(tequation, desc->equation.real);
		tequation->num_equations = desc->num_equations;
		catastrophe_set_equation(catastrophe, tequation);
		equation = tequation;
		break;
	}
	case CT_COMPLEX: {
		cmplx_equation_t *tequation;
		tequation = construct_cmplx_equation();
		if (!tequation)
			goto error_construct_equation;
		equation_set_function(tequation, desc->equation.cmplx);
		tequation->num_equations = desc->num_equations;
		catastrophe_set_equation(catastrophe, tequation);
		equation = tequation;
		break;
	}
	default:
		equation = NULL;
	}

	pair = catastrophe_search_alterable_params(catastrophe);
	if (!catastrophe_is_pair_correct(&pair))
		goto error_incorrect_pair;

	point_array = construct_point_array(
			catastrophe->parameter[pair.first].min_value,
			catastrophe->parameter[pair.first].max_value,
			catastrophe->parameter[pair.first].num_steps,
			catastrophe->parameter[pair.second].min_value,
			catastrophe->parameter[pair.second].max_value,
			catastrophe->parameter[pair.second].num_steps);
	if (!point_array)
		goto error_construct_point_array;

	catastrophe_set_point_array(catastrophe, point_array);

	catastrophe_set_calculate(catastrophe, desc->calculate);
	catastrophe_set_name(catastrophe, desc->sym_name);

	return catastrophe;

error_construct_point_array:
	err--;
error_incorrect_pair:
	err--;
	destruct_equation(equation);
error_construct_equation:
	err--;
error_bind_vars:
	err--;
error_bind_params:
	err--;
	destruct_catastrophe(catastrophe);
error:
	err--;
	WAVECAT_ERROR(err);
	return NULL;
}

