#include <kernel/core/point_array.h>
#include <kernel/integration/cmplx_runge_kutta.h>

#include <math.h>

/*
 * Edge catastrophe Fsub4. W = (V, V1), gamma = 2.
 */

enum parameters {
	LAMBDA_1 = 0,
	LAMBDA_2,
	LAMBDA_3,
	K
};

enum components {
	V = 0,
	V1
};

enum storage_entries {
	Ai = 0,
	Aid
};

static char *par_names[] = {"l1", "l2", "l3", "k", NULL};

void cmplx_catastrophe_Fsub4_function(
		const catastrophe_t *const catastrophe,
		const double t, const double complex *y,
		double complex *const f)
{
	double l1, l2, l3;
	double complex U01, U02, U22, U13;

	l1 = PARAM(LAMBDA_1) * t;
	l2 = PARAM(LAMBDA_2) * t;
	l3 = PARAM(LAMBDA_3) * t;

	U02 = 0.5 * (-PARAM(K)) * (I * (l2 * y[V] - I * l3 * y[V1]) + STORAGE_COMPLEX(Ai));

	U01 = 0.5 * PARAM(K) * (I * STORAGE_COMPLEX(Aid) - l2 * y[V1] + I/3.0 * l3 *
			(l1 * y[V] - I * l3 * U02));

	U22 = 0.5 * PARAM(K) / I * (1.0 + l2 * U02 + l3 * U01);

	U13 = -I/3.0 * (l1 * U02 - I * l3 * U22);

	f[V] = PARAM(LAMBDA_3) * U01;
	f[V1] = PARAM(LAMBDA_3) * U13;
}

static void cmplx_catastrophe_Frenaile_function(
		const catastrophe_t *const catastrophe,
		const double t, const double complex *y,
		double complex *const f)
{
	f[V] = -PARAM(K) * 0.5 * PARAM(LAMBDA_2) *
		(1.0 + I * PARAM(LAMBDA_2) * t * y[V]);
}

static void cmplx_catastrophe_Airy_function(
		const catastrophe_t *const catastrophe,
		const double t, const double complex *y,
		double complex *const f)
{
	double l1;
	double complex U11;

	l1 = PARAM(LAMBDA_1) * t;

	U11 = l1 / 3.0 * y[V];

	f[V]  = PARAM(LAMBDA_1) * y[V1];
	f[V1] = PARAM(LAMBDA_1) * U11;
}

static void calculate(catastrophe_t *const catastrophe,
		const unsigned int i, const unsigned int j)
{
	cmplx_equation_t *equation;
	point_array_t *point_array;

	double module;
	double phase;

	double complex F2;

	const double g13 = 2.678938534;
	const double g23 = 1.354117939;

	double divsqrt3 = 1.0 / sqrt(3.0);

	assert(catastrophe);

	equation = catastrophe->equation;
	point_array = catastrophe->point_array;

	assert(equation);
	assert(point_array);

	equation->initial_vector[V] = 0.5 * sqrt(M_PI) *
		cexp(I * PARAM(K) * M_PI / 4.0);
	equation_set_function(equation, cmplx_catastrophe_Frenaile_function);
	cmplx_runge_kutta(0.0, 1.0, 0.01, catastrophe);
	F2 = equation->resulting_vector[V];

	equation->initial_vector[V] = divsqrt3 * g13;
	equation->initial_vector[V1] = -divsqrt3 * g23;
	equation_set_function(equation, cmplx_catastrophe_Airy_function);
	cmplx_runge_kutta(0.0, 1.0, 0.01, catastrophe);

	STORAGE_COMPLEX(Ai) = equation->resulting_vector[V];
	STORAGE_COMPLEX(Aid) = equation->resulting_vector[V1];

	equation->initial_vector[V] = STORAGE_COMPLEX(Ai) * F2;
	equation->initial_vector[V1] = STORAGE_COMPLEX(Aid) * F2;
	equation_set_function(equation, cmplx_catastrophe_Fsub4_function);
	cmplx_runge_kutta(0.0, 1.0, 0.008, catastrophe);

	module = cabs(equation->resulting_vector[V]);
	phase = (180.0 / M_PI) * carg(equation->resulting_vector[V]);

	point_array->array[i][j].module = module;
	point_array->array[i][j].phase = phase;
}

static catastrophe_desc_t cmplx_catastrophe_Fsub4_desc = {
	.type = CT_COMPLEX,
	.sym_name = "Fsub4",
	.fabric = catastrophe_fabric,
	.num_parameters = 4,
	.par_names = par_names,
	.equation.cmplx = cmplx_catastrophe_Fsub4_function,
	.num_equations = 2,
	.calculate = calculate
};

static int cmplx_catastrophe_Fsub4_init(void) __attribute__ ((constructor));
static int cmplx_catastrophe_Fsub4_init(void)
{
	fprintf(stderr, "Catastrophe Fsub4 (complex) initialization.\n");
	register_catastrophe_desc(&cmplx_catastrophe_Fsub4_desc);
}
