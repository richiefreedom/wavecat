#include <kernel/core/point_array.h>
#include <kernel/integration/cmplx_runge_kutta.h>

#include <math.h>

/*
 * Corner catastrophe Asub1sup4. W = (V), gamma = Ng = 1.
 */

enum parameters {
	LAMBDA_1 = 0,
	LAMBDA_2,
	ALPHA,
	K_1,
	K_2
};

enum components {
	V = 0
};

enum storage_items {
	F2l1,
	F2l2,
	F2dl2
};

static char *par_names[] = {"l1", "l2", "a", "k1", "k2", NULL};

void cmplx_catastrophe_Asub1sup4_function(
		const catastrophe_t *const catastrophe,
		const double t, const double complex *y,
		double complex *const f)
{
	double l1, l2, a, D;
	double complex U0a;

	l1 = PARAM(LAMBDA_1);
	l2 = PARAM(LAMBDA_2);
	a = PARAM(ALPHA) * t;

	D = pow(a, 2) - 4.0 * PARAM(K_1) * PARAM(K_2);

	U0a = 1.0 / D * (
		-a * y[V] + I / D * (a * l2 - 2.0 * PARAM(K_2) * l1) *
		((a * l1 - 2.0 * PARAM(K_1) * l2) * y[V] +
		I * (2.0 * PARAM(K_1) * STORAGE_COMPLEX(F2l1) - a * STORAGE_COMPLEX(F2l2))) -
		2.0 * I * PARAM(K_2) * STORAGE_COMPLEX(F2dl2));

	f[V] = PARAM(ALPHA) * U0a;
}

void cmplx_catastrophe_Bsub2l1_function(
		const catastrophe_t *const catastrophe,
		const double t, const double complex *y,
		double complex *const f)
{
	f[V] = -PARAM(K_1) * 0.5 * PARAM(LAMBDA_1) *
		(1.0 + I * PARAM(LAMBDA_1) * t * y[V]);
}

void cmplx_catastrophe_Bsub2l2_function(
		const catastrophe_t *const catastrophe,
		const double t, const double complex *y,
		double complex *const f)
{
	f[V] = -PARAM(K_2) * 0.5 * PARAM(LAMBDA_2) *
		(1.0 + I * PARAM(LAMBDA_2) * t * y[V]);
}

static void calculate(catastrophe_t *const catastrophe,
		const unsigned int i, const unsigned int j)
{
	cmplx_equation_t *equation;
	point_array_t *point_array;

	double module;
	double phase;

	const double sqrt_pi = sqrt(M_PI);

	assert(catastrophe);

	equation = catastrophe->equation;
	point_array = catastrophe->point_array;

	assert(equation);
	assert(point_array);

	equation->initial_vector[V] = 0.5 * sqrt_pi *
		cexp(I * PARAM(K_1) * M_PI / 4.0);

	equation_set_function(equation, cmplx_catastrophe_Bsub2l1_function);
	cmplx_runge_kutta(0.0, 1.0, 0.01, catastrophe);

	STORAGE_COMPLEX(F2l1) = equation->resulting_vector[V];

	equation->initial_vector[V] = 0.5 * sqrt_pi *
		cexp(I * PARAM(K_2) * M_PI / 4.0);

	equation_set_function(equation, cmplx_catastrophe_Bsub2l2_function);

	cmplx_runge_kutta(0.0, 1.0, 0.01, catastrophe);
	STORAGE_COMPLEX(F2l2) = equation->resulting_vector[V];

	STORAGE_COMPLEX(F2dl2) = -PARAM(K_2) * 0.5 *
			(1.0 + I * PARAM(LAMBDA_2) * STORAGE_COMPLEX(F2l2));

	equation->initial_vector[V] = STORAGE_COMPLEX(F2l1) * STORAGE_COMPLEX(F2l2);
	equation_set_function(equation, cmplx_catastrophe_Asub1sup4_function);
	cmplx_runge_kutta(0.0, 1.0, 0.008, catastrophe);

	module = cabs(equation->resulting_vector[V]);
	phase = (180.0 / M_PI) * carg(equation->resulting_vector[V]);

	point_array->array[i][j].module = module;
	point_array->array[i][j].phase = phase;
}

static catastrophe_desc_t cmplx_catastrophe_Asub1sup4_desc = {
	.type = CT_COMPLEX,
	.sym_name = "Asub1sup4",
	.fabric = catastrophe_fabric,
	.num_parameters = 5,
	.par_names = par_names,
	.equation.cmplx = cmplx_catastrophe_Asub1sup4_function,
	.num_equations = 1,
	.calculate = calculate
};

static int cmplx_catastrophe_Asub1sup4_init(void) __attribute__ ((constructor));
static int cmplx_catastrophe_Asub1sup4_init(void)
{
	fprintf(stderr, "Catastrophe Asub1sup4 (complex) initialization.\n");
	register_catastrophe_desc(&cmplx_catastrophe_Asub1sup4_desc);
}
