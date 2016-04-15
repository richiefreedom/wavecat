#include <kernel/core/point_array.h>
#include <kernel/integration/cmplx_runge_kutta.h>

#include <math.h>

/* Components of the fundamental vector */
enum components {
	V = 0,
	V1
};

/* Parameters */
enum parameters {
	LAMBDA_1 = 0,
	LAMBDA_2,
	LAMBDA_3,
	ALPHA
};

/* Internal variables */
enum variables {
	/* Real parameters */
	K1 = 0,
	K2
};

enum storage_items {
	VB2L3 = 0,	/* Bsub2(l3) */
	VB3L1L2,	/* Bsub3(l1, l2) */
	DVB3L1L2,	/* Bsub3'(l1, l2) */
	DDVB3L1L2	/* Bsub3''(l1, l2) */
};

/* This arrays of strings will be used in variables binding. */
static char *par_names[] = {"l1", "l2", "l3", "a", NULL};
static char *var_names[] = {"k1", "k2", NULL};

static void Bsub2_function(
		const catastrophe_t *const catastrophe,
		const double t, const double complex *y,
		double complex *const f)
{
	double l3;

	l3 = PARAM(LAMBDA_3) * t;

	f[V] = -VAR(K1) * 0.5 * PARAM(LAMBDA_3) *
		(1.0 + I * PARAM(LAMBDA_3) * t * y[V]);
}

#define VAR_L(num) \
	(((catastrophe_t *) catastrophe)->variable[(num)].cur_value)

static void Bsub3_function(
		const catastrophe_t *const catastrophe,
		const double t, const double complex *y,
		double complex *const f)
{
	double l1, l2;
	double k = VAR(K2);
	complex double U11, U12;

	l1 = PARAM(LAMBDA_1) * t;
	l2 = PARAM(LAMBDA_2) * t;

	U11 = (k / 3.0) * (l1 * y[V] - 2.0 * I * l2 * y[V1] - I);
	U12 = -(k * I / 3.0) *
		(y[V] + l1 * y[V1] - 2.0 * I * l2 * U11);

	VAR_L(VB3L1L2) = f[V] = PARAM(LAMBDA_1) * y[V1] +
		PARAM(LAMBDA_2) * (-I * U11);
	VAR_L(DVB3L1L2) = f[V1] = PARAM(LAMBDA_1) * U11 +
		PARAM(LAMBDA_2) * U12;
	VAR_L(DDVB3L1L2) = (k / 3.0) * (l1 * f[V] - 2.0 * I * l2 * f[V1] - I);
}

void cmplx_catastrophe_Asub1Asub2Asub1Asub1_function(
		const catastrophe_t *const catastrophe,
		const double t, const double complex *y,
		double complex *const f)
{
	double l1, l2, l3, a;
	double complex U11, U03, U111;
	double complex V0a, V1a;

	l1 = PARAM(LAMBDA_1);
	l2 = PARAM(LAMBDA_2);
	l3 = PARAM(LAMBDA_3);
	a = PARAM(ALPHA) * t; /* Check it! */

	U03 = (I*VAR(K1)/2.0) *
		(I*STORAGE_COMPLEX(VB3L1L2) - l3*y[V] + I*a*y[V1]);
	U11 = (-VAR(K2)/3.0) *
		(I*STORAGE_COMPLEX(VB2L3) - l1*y[V] + 2.0*I*l2*y[V1] + I*a*U03);

	V0a = (VAR(K1)/2.0) *
		(I*STORAGE_COMPLEX(DVB3L1L2) - l3*y[V1] + I*a*U11);

	U111 = (-VAR(K2)/2.0) *
		(-y[V] - l1*y[V1] + 2*I*l2*U11 - a*V0a);
	V1a = (VAR(K1)/2.0) *
		(I*STORAGE_COMPLEX(DDVB3L1L2) - l3*U11 + I*a*U111);

	f[V] = V0a*PARAM(ALPHA);
	f[V1] = V1a*PARAM(ALPHA);
}

static void calculate(catastrophe_t *const catastrophe,
		const unsigned int i, const unsigned int j)
{
	cmplx_equation_t *equation;
	point_array_t *point_array;

	double module;
	double phase;

	const double sqrt_pi = sqrt(M_PI);
	const double g13 = 2.678938534;
	const double g23 = 1.354117939;

	assert(catastrophe);

	equation = catastrophe->equation;
	point_array = catastrophe->point_array;

	assert(equation);
	assert(point_array);

	/* Precalculate Bsub2(l3) */
	equation->initial_vector[V] = 0.5 * sqrt_pi *
		cexp(I * VAR(K1) * M_PI / 4.0);
	equation_set_function(equation, Bsub2_function);
	cmplx_runge_kutta(0.0, 1.0, 0.01, catastrophe);
	STORAGE_COMPLEX(VB2L3) = equation->resulting_vector[V];

	/* Precalculate Bsub3 */
	equation->initial_vector[V] = (1.0 / 3.0) * g13 *
		cexp(I * VAR(K2) * M_PI / 6.0);
	equation->initial_vector[V1] = (I / 3.0) * g23 *
		cexp(I * VAR(K2) * M_PI / 3.0);
	equation_set_function(equation, Bsub3_function);
	cmplx_runge_kutta(0.0, 1.0, 0.01, catastrophe);

	equation->initial_vector[V] = STORAGE_COMPLEX(VB2L3) * STORAGE_COMPLEX(VB3L1L2);
	equation->initial_vector[V1] = STORAGE_COMPLEX(VB2L3) * STORAGE_COMPLEX(DVB3L1L2);
	equation_set_function(equation,
			cmplx_catastrophe_Asub1Asub2Asub1Asub1_function);
	cmplx_runge_kutta(0.0, 1.0, 0.01, catastrophe);

	module = cabs(equation->resulting_vector[V]);
	phase = (180.0 / M_PI) * carg(equation->resulting_vector[V]);

	point_array->array[i][j].module = module;
	point_array->array[i][j].phase = phase;
}

static catastrophe_desc_t
	cmplx_catastrophe_Asub1Asub2Asub1Asub1_desc = {
	.type = CT_COMPLEX,
	.sym_name = "Asub1Asub2Asub1Asub1",
	.fabric = catastrophe_fabric,
	.num_parameters = 4,
	.num_variables = 6,
	.par_names = par_names,
	.var_names = var_names,
	.equation.cmplx = cmplx_catastrophe_Asub1Asub2Asub1Asub1_function,
	.num_equations = 2,
	.calculate = calculate
};

static int Asub1Asub2Asub1Asub1_init(void) __attribute__ ((constructor));
static int Asub1Asub2Asub1Asub1_init(void)
{
	fprintf(stderr,
	"Catastrophe Asub1Asub2Asub1Asub1 (complex) initialization.\n");
	register_catastrophe_desc(
		&cmplx_catastrophe_Asub1Asub2Asub1Asub1_desc);
}
