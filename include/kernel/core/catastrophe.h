#ifndef _WAVECAT_CATASTROPHE_H_
#define _WAVECAT_CATASTROPHE_H_

#include <kernel/core/config.h>
#include <kernel/core/pair.h>
#include <kernel/core/equation.h>
#include <kernel/core/cmplx_equation.h>
#include <kernel/core/point_array.h>
#include <kernel/adt/list.h>

#define MAX_NAME_LEN 80

#define PARAM(num)    (catastrophe->parameter[(num)].cur_value)
#define VAR(num)      (catastrophe->variable[(num)].cur_value)
#define STORAGE_REAL(num)     (((equation_t *)(catastrophe->equation))->storage[(num)])
#define STORAGE_COMPLEX(num)  (((cmplx_equation_t *)(catastrophe->equation))->storage[(num)])

struct parameter_s {
	double         cur_value;
	double         min_value;
	double         max_value;
	unsigned int   num_steps;
	char           sym_name[MAX_NAME_LEN];
};

struct variable_s {
	double cur_value;
	char   sym_name[MAX_NAME_LEN];
};

enum catastrophe_type_e {
	CT_REAL = 0,
	CT_COMPLEX,
};

typedef struct parameter_s         parameter_t;
typedef struct variable_s          variable_t;
typedef enum   catastrophe_type_e  catastrophe_type_t;

typedef struct catastrophe_s catastrophe_t;

typedef void (*catastrophe_calculate_t)(catastrophe_t *const catastrophe,
			const unsigned int i, const unsigned int j);

struct catastrophe_s {
	catastrophe_type_t    type;

	struct parameter_s    parameter[CONFIG_CAT_MAX_PARAMETERS];
	unsigned int          num_parameters;

	struct variable_s     variable[CONFIG_CAT_MAX_VARIABLES];
	unsigned int          num_variables;

	void                 *equation;
	point_array_t        *point_array;
	const char           *sym_name;

	catastrophe_calculate_t calculate;
};

/**
 * construct_catastrophe() - a simple constructor of catastrophe objects.
 *
 * Allocates an object and initializes it with zeroes. All other functionality
 * is done in the fabric method.
 *
 * Returns a pointer to new catastrophe_t object or NULL otherwise.
 */
static inline catastrophe_t *construct_catastrophe(void)
{
	catastrophe_t *cat;
	cat = malloc(sizeof(*cat));
	if (!cat)
		return NULL;
	memset(cat, 0, sizeof(*cat));
	return cat;
}

static inline void destruct_catastrophe(catastrophe_t *cat)
{
	if (!cat)
		return;

	if (cat->equation)
		destruct_equation(cat->equation);
	if (cat->point_array)
		destruct_point_array(cat->point_array);

	free(cat);
}

#define catastrophe_set_name(cat, name) (cat)->sym_name = (name)
#define catastrophe_set_equation(cat, equ) (cat)->equation = (equ)
#define catastrophe_set_calculate(cat, calc) \
	(cat)->calculate = (calc)
#define catastrophe_set_point_array(cat, point_array) \
	(cat)->point_array = (point_array)
#define catastrophe_get_real_equation(cat) (cat)->equation
#define catastrophe_get_cmplx_equation(cat) (cat)->equation

static inline uint_pair_t
catastrophe_search_alterable_params(catastrophe_t *cat)
{
	uint_pair_t pair;
	unsigned int index[2];
	unsigned int index_no;
	unsigned int i;

	memset(&pair, 0, sizeof(pair));

	if (!cat)
		return pair;

	for (i = 0, index_no = 0;
			i < cat->num_parameters && index_no < 2;
			i++) {
		parameter_t *par;
		par = &cat->parameter[i];
		if (par->num_steps != 0 && par->min_value != par->max_value) {
			index[index_no++] = i;
		}
	}

	if (index_no != 2)
		return pair;

	pair.first = index[0];
	pair.second = index[1];
	return pair;
}

static inline int
catastrophe_is_pair_correct(uint_pair_t *pair)
{
	return (pair->first != pair->second);
}

int catastrophe_loop(catastrophe_t *const catastrophe);

typedef struct catastrophe_desc_s catastrophe_desc_t;
typedef catastrophe_t *(*catastrophe_fabric_t)(catastrophe_desc_t *desc,
		parameter_t *parameter, variable_t *variable);

struct catastrophe_desc_s {
	catastrophe_type_t   type;

	list_head_t          list;
	char                *sym_name;
	catastrophe_fabric_t fabric;

	unsigned int         num_parameters;
	unsigned int         num_variables;

	char               **par_names;
	char               **var_names;

	union {
		equation_function_t        real;
		cmplx_equation_function_t  cmplx;
	} equation;

	unsigned int         num_equations;

	catastrophe_calculate_t calculate;
};

catastrophe_t *catastrophe_fabric(catastrophe_desc_t *desc,
		parameter_t *parameter, variable_t *variable);

void register_catastrophe_desc(catastrophe_desc_t *cd);
void unregister_catastrophe_desc(catastrophe_desc_t *cd);
catastrophe_desc_t *find_catastrophe_desc(const char *sym_name);

#endif /* _WAVECAT_CATASTROPHE_H_ */
