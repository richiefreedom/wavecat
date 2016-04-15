#include <kernel/core/point_array.h>
#include <kernel/integration/cmplx_runge_kutta.h>

#include <math.h>

enum parameters {
	LAMBDA_1 = 0,
	LAMBDA_2,
	LAMBDA_3
};

enum variables {
	B = 0
};

enum components {
	V = 0,
	V1,
	V2,
	V3
};

static char *par_names[] = {"l1", "l2", "l3", NULL};
static char *var_names[] = {"b", NULL};

void cmplx_catastrophe_Dsub4_function(
		const catastrophe_t *const catastrophe,
		const double t, const double complex *y,
		double complex *const f)
{
	double l1, l2, l3;
	double complex U11, U12, U13,
	               U21, U22, U23,
		       U31, U32, U33,
	               U41, U42, U43;

	l1 = PARAM(LAMBDA_1) * t;
	l2 = PARAM(LAMBDA_2) * t;
	l3 = PARAM(LAMBDA_3) * t;

	U11 = y[V1]; U12 = y[V2]; U13 = y[V3];

	U21 = VAR(B) * (l2 * y[V] - 3.0 * I * y[V3] - 2.0 * I * l3 * y[V2]);
	U22 = U31 = 0.5 * l1 * y[V] * VAR(B);
	U23 = U41 = 0.5 * (-I) * l1 * y[V2]  * VAR(B);

	U32 = I * y[V3];
	U33 = U42 = -I/3.0 * (0.5 * y[V] + l2 * y[V2] + 2.0 * l3 * y[V3] -
		0.5 * l1 * y[V1]);
	
	U43 = -1.0/3.0 * (3.0/2.0 * y[V2] + l2 * I * y[V3] + 2.0 * l3 * U42 -
		0.5 * l1 * U22);
	
	f[V] = U11 * PARAM(LAMBDA_1) + U12 * PARAM(LAMBDA_2) + U13 *
		PARAM(LAMBDA_3);
	f[V1] = U21 * PARAM(LAMBDA_1) + U22 * PARAM(LAMBDA_2) + U23 *
		PARAM(LAMBDA_3);
	f[V2] = U31 * PARAM(LAMBDA_1) + U32 * PARAM(LAMBDA_2) + U33 *
		PARAM(LAMBDA_3);
	f[V3] = U41 * PARAM(LAMBDA_1) + U42 * PARAM(LAMBDA_2) + U43 *
		PARAM(LAMBDA_3);
}

static void calculate(catastrophe_t *const catastrophe,
		const unsigned  int i, const unsigned int j)
{
	cmplx_equation_t *equation;
	point_array_t *point_array;

	/* Gamma function's precalculated values */
	const double g16 = 5.566316001;
	const double g56 = 1.128787029;

	/* Precalculate some trigonometric functions */
	double cp12 = cos(M_PI / 12.0);
	double c5p12 = cos(5.0 * M_PI / 12.0);

	/* Precalculate the necessary square root */
	double sqrt2p = sqrt(2.0 * M_PI);

	double module;
	double phase;

	assert(catastrophe);

	equation = catastrophe->equation;
	point_array = catastrophe->point_array;

	assert(equation);
	assert(point_array);

	equation->initial_vector[V] = sqrt2p / 3.0 * g16 *
		(cp12 - VAR(B) * cp12);
	equation->initial_vector[V1] = 0;
	equation->initial_vector[V2] = -M_PI / 3.0 * (1.0 + VAR(B));
	equation->initial_vector[V3] = I * sqrt2p / 3.0 * g56 *
		(c5p12 - VAR(B) * c5p12);

	cmplx_runge_kutta(0.0, 1.0, 0.01, catastrophe);

	module = cabs(equation->resulting_vector[V]);
	phase = (180.0 / M_PI) * carg(equation->resulting_vector[V]);

	point_array->array[i][j].module = module;
	point_array->array[i][j].phase = phase;
}

static catastrophe_desc_t cmplx_catastrophe_Dsub4_desc = {
	.type = CT_COMPLEX,
	.sym_name = "Dsub4",
	.fabric = catastrophe_fabric,
	.num_parameters = 3,
	.num_variables = 1,
	.par_names = par_names,
	.var_names = var_names,
	.equation.cmplx = cmplx_catastrophe_Dsub4_function,
	.num_equations = 4,
	.calculate = calculate
};

static int cmplx_catastrophe_Dsub4_init(void) __attribute__ ((constructor));
static int cmplx_catastrophe_Dsub4_init(void)
{
	fprintf(stderr, "Catastrophe Dsub4 (complex) initialization.\n");
	register_catastrophe_desc(&cmplx_catastrophe_Dsub4_desc);
}
