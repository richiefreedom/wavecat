#include <wavecat/point_array.h>
#include <lib/integration/cmplx_runge_kutta.h>

#include <math.h>

enum parameters {
	LAMBDA_1 = 0,
	LAMBDA_2
};

enum components {
	V = 0,
	V1
};

enum variables {
	K = 0,
};

enum storage_entries {
	airy = 0,
	dairy
};

static char *par_names[] = {"l1", "l2", NULL};
static char *var_names[] = {"k", NULL};

void cmplx_catastrophe_Csub3_function(
		const catastrophe_t *const catastrophe,
		const double t, const double complex *y,
		double complex *const f)
{
	double l1, l2;
	double complex U01, U02;

	l1 = PARAM(LAMBDA_1) * t;
	l2 = PARAM(LAMBDA_2) * t;

	U01 = I * (I * STORAGE_COMPLEX(airy) - l2 * y[V]);	
	U02 = I * (3.0 * VAR(K) * I * (I * STORAGE_COMPLEX(dairy) - l2 * U01) - l1 * y[V]);

	f[V] = PARAM(LAMBDA_1) * U01 + PARAM(LAMBDA_2) * U02;
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
		const unsigned  int i, const unsigned int j)
{
	cmplx_equation_t *equation;
	point_array_t *point_array;

	double module;
	double phase;

	const double g13 = 2.678938534;
	const double g23 = 1.354117939;

	double divsqrt3 = 1.0 / sqrt(3.0);

	assert(catastrophe);

	equation = catastrophe->equation;
	point_array = catastrophe->point_array;

	assert(equation);
	assert(point_array);

	equation->initial_vector[V] = divsqrt3 * g13;
	equation->initial_vector[V1] = -divsqrt3 * g23;
	equation->num_equations = 2;
	equation_set_function(equation, cmplx_catastrophe_Airy_function);
	cmplx_runge_kutta(0.0, 1.0, 0.01, catastrophe);
	STORAGE_COMPLEX(airy) = equation->resulting_vector[V];
	STORAGE_COMPLEX(dairy) = equation->resulting_vector[V1];

	equation->initial_vector[V] = M_PI * (1.0 - VAR(K)/3.0);
	/* A simple hack to reduce calculation time. */
	equation->num_equations = 1;
	equation_set_function(equation, cmplx_catastrophe_Csub3_function);
	cmplx_runge_kutta(0.0, 1.0, 0.01, catastrophe);

	module = cabs(equation->resulting_vector[V]);
	phase = (180.0 / M_PI) * carg(equation->resulting_vector[V]);

	point_array->array[i][j].module = module;
	point_array->array[i][j].phase = phase;
}

static catastrophe_desc_t cmplx_catastrophe_Csub3_desc = {
	.type = CT_COMPLEX,
	.sym_name = "Csub3",
	.fabric = catastrophe_fabric,
	.num_parameters = 2,
	.num_variables = 3,
	.par_names = par_names,
	.var_names = var_names,
	.equation.cmplx = cmplx_catastrophe_Csub3_function,
	.num_equations = 2,
	.calculate = calculate
};

static int cmplx_catastrophe_Csub3_init(void) __attribute__ ((constructor));
static int cmplx_catastrophe_Csub3_init(void)
{
	fprintf(stderr, "Catastrophe Csub3 (complex) initialization.\n");
	register_catastrophe_desc(&cmplx_catastrophe_Csub3_desc);
}
