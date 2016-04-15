#include <kernel/core/catastrophe.h>
#include <kernel/core/point_array.h>
#include <kernel/integration/runge_kutta.h>

#include <math.h>

enum parameters {
	LAMBDA_1 = 0,
	LAMBDA_2,
	ALPHA
};

enum variables {
	VARIABLE_K1 = 0,
	VARIABLE_K2
};

enum storage_entries {
	FL1R = 0,
	FL1M,
	FL2R,
	FL2M,
	FSR,
	FSM
};

enum components {
	VR = 0,
	VM
};

static char *par_names[] = {"l1", "l2", "a", NULL};
static char *var_names[] = {"k1", "k2", NULL};

static void function_1(const catastrophe_t *const catastrophe,
		const double t, const double *y, double *const f)
{
	f[0] = -VAR(VARIABLE_K1) * 0.5 * PARAM(LAMBDA_1) *
		(1.0 - PARAM(LAMBDA_1) * t * y[1]);
	f[1] = -VAR(VARIABLE_K1) * 0.5 * PARAM(LAMBDA_1) * PARAM(LAMBDA_1) *
		t * y[0];
}

static void function_2(const catastrophe_t *const catastrophe,
		const double t, const double *y, double *const f)
{
	f[0] = -VAR(VARIABLE_K2) * 0.5 * PARAM(LAMBDA_2) *
		(1.0 - PARAM(LAMBDA_2) * t * y[1]);
	f[1] = -VAR(VARIABLE_K2) * 0.5 * PARAM(LAMBDA_2) * PARAM(LAMBDA_2) *
		t * y[0];
}

static void catastrophe_Asub1sup4_function(
		const catastrophe_t *const catastrophe,
		const double t, const double *y, double *const f)
{
	double L1, L2, alpha, d;

	alpha = PARAM(ALPHA) * t;

	d = alpha * alpha - 4.0 * VAR(VARIABLE_K1) * VAR(VARIABLE_K2);

	L1 = alpha * PARAM(LAMBDA_2) - 2.0 * VAR(VARIABLE_K2) * PARAM(LAMBDA_1);
	L2 = alpha * PARAM(LAMBDA_1) - 2.0 * VAR(VARIABLE_K1) * PARAM(LAMBDA_2);

	f[0] = PARAM(ALPHA) * ((1.0 / d) * (-alpha * y[0] - (L1 * L2 * y[1] / d)
		- (L1 * (2.0 * VAR(VARIABLE_K1) * STORAGE_REAL(FL1R) - alpha * STORAGE_REAL(FL2R)) / d) +
		2.0 * VAR(VARIABLE_K2) * STORAGE_REAL(FSM)));
	f[1] = PARAM(ALPHA) * ((1.0 / d) * (-alpha * y[1] + (L1 * L2 * y[0] / d)
		- (L1 * (2.0 * VAR(VARIABLE_K1) * STORAGE_REAL(FL1M) - alpha * STORAGE_REAL(FL2M)) / d) -
		2.0 * VAR(VARIABLE_K2) * STORAGE_REAL(FSR)));
}

static void calculate(catastrophe_t *const catastrophe,
		const unsigned  int i, const unsigned int j)
{
	equation_t *equation;
	point_array_t *point_array;

	assert(catastrophe);

	const double sqrt_pi = sqrt(M_PI);

	/* Precalculate some trigonometric functions */
	double fii = M_PI / 4.0;
	double cos1 = cos(VAR(VARIABLE_K1) * M_PI / 4.0);
	double cos2 = cos(VAR(VARIABLE_K2) * M_PI / 4.0);
	double sin1 = sin(VAR(VARIABLE_K1) * M_PI / 4.0);
	double sin2 = sin(VAR(VARIABLE_K2) * M_PI / 4.0);

	double module;
	double phase;

	equation = catastrophe->equation;
	point_array = catastrophe->point_array;

	assert(equation);
	assert(point_array);

	equation->initial_vector[0] = 0.5 * sqrt_pi * cos1;
	equation->initial_vector[1] = 0.5 * sqrt_pi * sin1;
	equation_set_function(equation, function_1);

	runge_kutta(0.0, 1.0, 0.01, catastrophe);

	STORAGE_REAL(FL1R) = equation->resulting_vector[0];
	STORAGE_REAL(FL1M) = equation->resulting_vector[1];

	equation->initial_vector[0] = 0.5 * sqrt_pi * cos2;
	equation->initial_vector[1] = 0.5 * sqrt_pi * sin2;
	equation_set_function(equation, function_2);

	runge_kutta(0.0, 1.0, 0.01, catastrophe);

	STORAGE_REAL(FL2R) = equation->resulting_vector[0];
	STORAGE_REAL(FL2M) = equation->resulting_vector[1];

	equation->initial_vector[0] = STORAGE_REAL(FL1R) * STORAGE_REAL(FL2R) -
		STORAGE_REAL(FL1M) * STORAGE_REAL(FL2M);
	equation->initial_vector[1] = STORAGE_REAL(FL1R) * STORAGE_REAL(FL2M) +
		STORAGE_REAL(FL1M) * STORAGE_REAL(FL2R);
	equation_set_function(equation, catastrophe_Asub1sup4_function);

	STORAGE_REAL(FSR) = -0.5 * VAR(VARIABLE_K2) * (1.0 - PARAM(LAMBDA_2) *
				STORAGE_REAL(FL2M));
	STORAGE_REAL(FSM) = -0.5 * VAR(VARIABLE_K2) * (PARAM(LAMBDA_2) *
			STORAGE_REAL(FL2R));

	runge_kutta(0.0, 1.0, 0.01, catastrophe);

	module = sqrt(equation->resulting_vector[0] *
			equation->resulting_vector[0] +
			equation->resulting_vector[1] *
			equation->resulting_vector[1]);
	phase = (180.0 / M_PI) * atan2(equation->resulting_vector[0],
			equation->resulting_vector[1]);

	point_array->array[i][j].module = module;
	point_array->array[i][j].phase = phase;
}

static catastrophe_desc_t catastrophe_Asub1sup4_desc = {
	.type = CT_REAL,
	.sym_name = "Asub1sup4",
	.fabric = catastrophe_fabric,
	.num_parameters = 3,
	.num_variables = 8,
	.par_names = par_names,
	.var_names = var_names,
	.equation.real = catastrophe_Asub1sup4_function,
	.num_equations = 2,
	.calculate = calculate
};

static int catastrophe_Asub1sup4_init(void) __attribute__ ((constructor));
static int catastrophe_Asub1sup4_init(void)
{
	fprintf(stderr, "Catastrophe Asub1sup4 initialization.\n");
	register_catastrophe_desc(&catastrophe_Asub1sup4_desc);
}
