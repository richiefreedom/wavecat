#include <kernel/core/point_array.h>
#include <kernel/integration/cmplx_runge_kutta.h>

#include <math.h>

enum parameters {
	LAMBDA_1 = 0,
	LAMBDA_2
};

enum components {
	V = 0,
	V1,
	V2
};

static char *par_names[] = {"l1", "l2", NULL};

void cmplx_catastrophe_Asub3_function(
		const catastrophe_t *const catastrophe,
		const double t, const double complex *y,
		double complex *const f)
{
	double l1, l2;
	double param1, param2;
	double complex i21, i22;

	param1 = catastrophe->parameter[LAMBDA_1].cur_value;
	param2 = catastrophe->parameter[LAMBDA_2].cur_value;

	l1 = param1 * t;
	l2 = param2 * t;

	i21 = 0.25 * (l1 * y[V] - 2.0 * I * l2 * y[V1]);
	i22 = -0.25 * I * (y[V] + l1 * y[V1] + 2.0 * l2 * y[V2]);

	f[V]  = param1 * y[V1] + param2 * y[V2];
	f[V1] = I * y[V2] * param1 + i21 * param2;
	f[V2] = param1 * i21 + param2 * i22;
}

static void calculate(catastrophe_t *const catastrophe,
		const unsigned  int i, const unsigned int j)
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

	equation->initial_vector[V] = 0.5 * g14 * cexp(I * M_PI / 8.0);
	equation->initial_vector[V1] = 0;
	equation->initial_vector[V2] = 0.5 * I * g34 *
		cexp(I * 3.0 * M_PI / 8.0);

	cmplx_runge_kutta(0.0, 1.0, 0.01, catastrophe);

	module = cabs(equation->resulting_vector[V]);
	phase = (180.0 / M_PI) * carg(equation->resulting_vector[V]);

	point_array->array[i][j].module = module;
	point_array->array[i][j].phase = phase;
}

static catastrophe_desc_t cmplx_catastrophe_Asub3_desc = {
	.type = CT_COMPLEX,
	.sym_name = "Asub3",
	.fabric = catastrophe_fabric,
	.num_parameters = 2,
	.num_variables = 0,
	.par_names = par_names,
	.equation.cmplx = cmplx_catastrophe_Asub3_function,
	.num_equations = 3,
	.calculate = calculate
};

static int cmplx_catastrophe_Asub3_init(void) __attribute__ ((constructor));
static int cmplx_catastrophe_Asub3_init(void)
{
	fprintf(stderr, "Catastrophe Asub3 (complex) initialization.\n");
	register_catastrophe_desc(&cmplx_catastrophe_Asub3_desc);
}
