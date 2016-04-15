#include <kernel/core/point_array.h>
#include <kernel/integration/cmplx_runge_kutta.h>

#include <math.h>

enum parameters {
	LAMBDA_1 = 0,
	LAMBDA_2
};

enum variables {
	K = 0
};

enum components {
	V = 0,
	V1
};

static char *par_names[] = {"l1", "l2", NULL};
static char *var_names[] = {"k", NULL};

void cmplx_catastrophe_Bsub3_function(
		const catastrophe_t *const catastrophe,
		const double t, const double complex *y,
		double complex *const f)
{
	double l1, l2;
	complex double U11, U12;

	l1 = PARAM(LAMBDA_1) * t;
	l2 = PARAM(LAMBDA_2) * t;

	U11 = (VAR(K) / 3.0) * (l1 * y[V] - 2.0 * I * l2 * y[V1] - I);
	U12 = -(VAR(K) * I / 3.0) *
		(y[V] + l1 * y[V1] - 2.0 * I * l2 * U11);

	f[V] = PARAM(LAMBDA_1) * y[V1] + PARAM(LAMBDA_2) * (-I * U11);
	f[V1] = PARAM(LAMBDA_1) * U11 + PARAM(LAMBDA_2) * U12;
}

static void calculate(catastrophe_t *const catastrophe,
		const unsigned int i, const unsigned int j)
{
	cmplx_equation_t *equation;
	point_array_t *point_array;

	/* Gamma function's precalculated values */
	const double g13 = 2.678938534;
	const double g23 = 1.354117939;

	double module;
	double phase;

	assert(catastrophe);

	equation = catastrophe->equation;
	point_array = catastrophe->point_array;

	assert(equation);
	assert(point_array);

	/* Calculate Bsub3 */
	equation->initial_vector[V] = (1.0 / 3.0) * g13 *
		cexp(I * VAR(K) * M_PI / 6.0);
	equation->initial_vector[V1] = (I / 3.0) * g23 *
		cexp(I * VAR(K) * M_PI / 3.0);

	cmplx_runge_kutta(0.0, 1.0, 0.01, catastrophe);

	module = cabs(equation->resulting_vector[V]);
	phase = (180.0 / M_PI) * carg(equation->resulting_vector[V]);

	point_array->array[i][j].module = module;
	point_array->array[i][j].phase = phase;
}

static catastrophe_desc_t cmplx_catastrophe_Bsub3_desc = {
	.type = CT_COMPLEX,
	.sym_name = "Bsub3",
	.fabric = catastrophe_fabric,
	.num_parameters = 2,
	.num_variables = 1,
	.par_names = par_names,
	.var_names = var_names,
	.equation.cmplx = cmplx_catastrophe_Bsub3_function,
	.num_equations = 2,
	.calculate = calculate
};

static int cmplx_catastrophe_Bsub3_init(void) __attribute__ ((constructor));
static int cmplx_catastrophe_Bsub3_init(void)
{
	fprintf(stderr, "Catastrophe Bsub3 (complex) initialization.\n");
	register_catastrophe_desc(&cmplx_catastrophe_Bsub3_desc);
}
