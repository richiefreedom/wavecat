#include <kernel/core/point_array.h>
#include <kernel/integration/cmplx_runge_kutta.h>

#include <math.h>

/*
 * Edge catastrophe Csub4. W = (V), gamma = 1.
 */

enum parameters {
	LAMBDA_1 = 0,
	LAMBDA_2,
	LAMBDA_3,
};

enum components {
	V = 0,
	V1,
	V2
};

enum variables {
	K = 0
};

enum storage_entries {
	Asub3 = 0,
	Asub3dl1,
	Asub3dl2
};

static char *par_names[] = {"l1", "l2", "l3", NULL};
static char *var_names[] = {"k", NULL};

void cmplx_catastrophe_Asub3_function(
		const catastrophe_t *const catastrophe,
		const double t, const double complex *y,
		double complex *const f);

void cmplx_catastrophe_Csub4_function(
		const catastrophe_t *const catastrophe,
		const double t, const double complex *y,
		double complex *const f)
{
	double l1, l2, l3;
	double complex U01, U02, U03, U12;

	l1 = PARAM(LAMBDA_1) * t;
	l2 = PARAM(LAMBDA_2) * t;
	l3 = PARAM(LAMBDA_3) * t;

	U01 = I * (I * STORAGE_COMPLEX(Asub3) - l3 * y[V]);
	U02 = I * STORAGE_COMPLEX(Asub3dl1) - l3 * U01;
	U12 = I * (I * STORAGE_COMPLEX(Asub3dl2) - l3 * U02);
	U03 = I * (4.0 * VAR(K) * U12 + 2.0 * I * l2 * U01 - l1 * y[V]);

	f[V] =  PARAM(LAMBDA_1) * U01 +
		PARAM(LAMBDA_2) * U02 +
		PARAM(LAMBDA_3) * U03;
}

static void calculate(catastrophe_t *const catastrophe,
		const unsigned int i, const unsigned int j)
{
	cmplx_equation_t *equation;
	point_array_t *point_array;

	/* Gamma function's precalculated values */
	const double g14 = 3.625609908;
	const double g34 = 1.225416702;

	double module;
	double phase;

	assert(catastrophe);

	equation = catastrophe->equation;
	point_array = catastrophe->point_array;

	assert(equation);
	assert(point_array);

	/* Calculate Pearcey function */
	equation->initial_vector[V] = 0.5 * g14 * cexp(I * M_PI / 8.0);
	equation->initial_vector[V1] = 0;
	equation->initial_vector[V2] = 0.5 * I * g34 *
		cexp(I * 3.0 * M_PI / 8.0);
	equation_set_function(equation, cmplx_catastrophe_Asub3_function);
	cmplx_runge_kutta(0.0, 1.0, 0.01, catastrophe);
	STORAGE_COMPLEX(Asub3) = equation->resulting_vector[V];
	STORAGE_COMPLEX(Asub3dl1) = equation->resulting_vector[V1];
	STORAGE_COMPLEX(Asub3dl2) = equation->resulting_vector[V2];

	/* Calculate Csub4 */
	equation->initial_vector[V] = M_PI;
	equation_set_function(equation, cmplx_catastrophe_Csub4_function);
	cmplx_runge_kutta(0.0, 1.0, 0.01, catastrophe);

	module = cabs(equation->resulting_vector[V]);
	phase = (180.0 / M_PI) * carg(equation->resulting_vector[V]);

	point_array->array[i][j].module = module;
	point_array->array[i][j].phase = phase;
}

static catastrophe_desc_t cmplx_catastrophe_Csub4_desc = {
	.type = CT_COMPLEX,
	.sym_name = "Csub4",
	.fabric = catastrophe_fabric,
	.num_parameters = 3,
	.num_variables = 4,
	.par_names = par_names,
	.var_names = var_names,
	.equation.cmplx = cmplx_catastrophe_Csub4_function,
	.num_equations = 3,
	.calculate = calculate
};

static int cmplx_catastrophe_Csub4_init(void) __attribute__ ((constructor));
static int cmplx_catastrophe_Csub4_init(void)
{
	fprintf(stderr, "Catastrophe Csub4 (complex) initialization.\n");
	register_catastrophe_desc(&cmplx_catastrophe_Csub4_desc);
}
