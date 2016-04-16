#include <kernel/core/catastrophe.h>
#include <kernel/core/equation.h>
#include <kernel/core/cmplx_equation.h>
#include <string.h>
#include <assert.h>

static DECLARE_LIST_HEAD(catastrophe_desc_list);

void register_catastrophe_desc(catastrophe_desc_t *cd)
{
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

	point_array_t *pa = catastrophe->point_array;

	for (i = 1; i <= p1_steps; i++) {
		/* Calculate the current value of the parameter */
		catastrophe->parameter[p1_idx].cur_value =
			p1_min + (i-1) * (p1_max - p1_min) / (p1_steps - 1);
		for (j = 1; j <= p2_steps; j++) {
			catastrophe->parameter[p2_idx].cur_value =
				p2_min + (j-1) *
				(p2_max - p2_min) /
				(p2_steps - 1);

			catastrophe->calculate(catastrophe, i - 1, j - 1);

			/* 
			 * Check the result of calculation in the point.
			 * In the case of infinum value the calculations
			 * must be stopped.
			 */
			if (pa->array[i-1][j-1].module > 100 ||
				pa->array[i-1][j-1].module < -100) {
				WAVECAT_ERROR(-1);
				return -1;
			}
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

static int bind_variable_names(catastrophe_desc_t *desc,
		catastrophe_t *cat, variable_t *var)
{
	unsigned int i, j;

	for (i = 0; desc->var_names[i] != NULL; i++) {
		assert(i < desc->num_variables);
		for (j = 0; j < desc->num_variables; j++) {
			if (strcmp(var[j].sym_name, desc->var_names[i]) == 0) {
				cat->variable[i] = var[j];
				break;
			}
		}
		if (j == desc->num_variables) {
			fprintf(stderr, "Error: unbound variable %s.\n",
					desc->var_names[i]);
			CGI_ERROR("Not all variables are bound");
			return -1;
		}
	}

	cat->num_variables = desc->num_variables;

	return 0;
}

catastrophe_t *catastrophe_fabric(catastrophe_desc_t *desc,
		parameter_t *parameter, variable_t *variable)
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

	if (desc->par_names) {
		if (bind_parameter_names(desc, catastrophe, parameter))
			goto error_bind_params;
	} else {
		for (i = 0; i < desc->num_parameters; i++)
			catastrophe->parameter[i] = parameter[i];
		catastrophe->num_parameters = desc->num_parameters;
	}

	if (desc->var_names) {
		if (bind_variable_names(desc, catastrophe, variable))
			goto error_bind_vars;
	} else {
		for (i = 0; i < desc->num_variables; i++)
			catastrophe->variable[i] = variable[i];
		catastrophe->num_variables = desc->num_variables;
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
