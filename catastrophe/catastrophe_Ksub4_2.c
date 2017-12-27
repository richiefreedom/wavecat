#include <kernel/core/catastrophe.h>
#include <kernel/core/point_array.h>
#include <kernel/integration/runge_kutta.h>

#include <math.h>

enum parameters {
	LAMBDA_1 = 0,
	LAMBDA_2,
	LAMBDA_3,
	LAMBDA_4,
	ALPHA,
	K_1,
	K_2
};

enum storage_entries {
	FP0R,
	FP0M,
	FP1R,
	FP1M,
	FP2R,
	FP2M,
	V12R,
	V12M,
	V13R,
	V13M,
	V22R,
	V22M
};

static char *par_names[] = {"l1", "l2", "l3", "l4", "a", "k1", "k2", NULL};

static void function_1(const catastrophe_t *const catastrophe,
		const double t, const double *y, double *const f)
{
	double l3;

	l3 = PARAM(LAMBDA_3) * t;

	f[0] = -PARAM(K_1) * 0.5 * PARAM(LAMBDA_3) * (1.0 - l3 * y[1]);
	f[1] = -PARAM(K_1) * 0.5 * PARAM(LAMBDA_3) * l3 * y[0];
}

static void function_2(const catastrophe_t *const catastrophe,
		const double t, const double *y, double *const f)
{
	double l1, l2;
	double i21r, i21m, i22r, i22m;

	l1 = PARAM(LAMBDA_1) * t;
	l2 = PARAM(LAMBDA_2) * t;

	i21r = 0.25 * (l1 * y[0] + 2.0 * l2 * y[3]);
	i21m = 0.25 * (l1 * y[1] - 2.0 * l2 * y[2]);
	i22r = 0.25 * (y[1] + l1 * y[3] + 2.0 * l2 * y[5]);
	i22m = 0.25 * (-y[0] - l1 * y[2] - 2.0 * l2 * y[4]);

	f[0] = PARAM(LAMBDA_1) * y[2] + PARAM(LAMBDA_2) * y[4];
	f[1] = PARAM(LAMBDA_1) * y[3] + PARAM(LAMBDA_2) * y[5];

	f[2] = -PARAM(LAMBDA_1) * y[5] + PARAM(LAMBDA_2) * i21r;
	f[3] = PARAM(LAMBDA_1) * y[4] + PARAM(LAMBDA_2) * i21m;

	f[4] = PARAM(LAMBDA_1) * i21r + PARAM(LAMBDA_2) * i22r;
	f[5] = PARAM(LAMBDA_1) * i21m + PARAM(LAMBDA_2) * i22m;
}

static void catastrophe_Ksub4_2_function(
		const catastrophe_t *const catastrophe,
		const double t, const double *y, double *const f)
{
	double l1, l2, l3, l4, a;
	double d, U04r, U04m, U14r, U14m, U24r, U24m;
	double u13r, u13m, U0Ar, U0Am, U1Ar, U1Am, U2Ar, U2Am, u12r, u12m;
	double u112r, u112m, u122r, u122m, u03r, u03m, u1122r, u1122m;

	l1 = PARAM(LAMBDA_1);
	l2 = PARAM(LAMBDA_2);
	l3 = PARAM(LAMBDA_3);
	l4 = PARAM(LAMBDA_4) * t;
	a = PARAM(ALPHA) * t;

	d = a * a - 4.0 * PARAM(K_1) * PARAM(K_2);

	/* U03 */
	u03r = -0.5 *
		(-l3 * y[1] + l4 * y[2] + a * y[4] + STORAGE_REAL(FP0R));
	u03m = -0.5 *
		(l3 * y[0] + l4 * y[3] + a * y[5] + STORAGE_REAL(FP0M));

	/* U12 */
	u12r = (1.0 / d) *
		(-(2.0 * l2 - a * l3) * y[3] - l1 * y[0] + l4 * a * y[5]
		 - a * STORAGE_REAL(FP1R) - l4 * u03m);
	u12m = (1.0 / d) *
		((2.0 * l2 - a * l3) * y[2] - l1 * y[1] - l4 * a * y[4]
		 - a * STORAGE_REAL(FP1M) + l4 * u03r);

	/* U13 */
	u13r = -0.5 *
		(-l3 * y[3] - l4 * y[5] + a * u12r + STORAGE_REAL(FP1R));
	u13m = -0.5 *
		(l3 * y[2] + l4 * y[4] + a * u12m + STORAGE_REAL(FP1M));

	/* U112 */
	u112r = (1.0 / d) *
		((-2.0 * l2 + a * l3) * y[4] - y[0] - l1 * y[2] +
		 a * l4 * u12m - l4 * u13m + a * STORAGE_REAL(FP2M));
	u112m = (1.0 / d) *
		((-2.0 * l2 + a * l3) * y[5] - y[1] - l1 * y[3] -
		 a * l4 * u12r + l4 * u13r - a * STORAGE_REAL(FP2R));

	/* U14 */
	U14r = -0.5 *
		(-l3 * y[5] + l4 * u12r + a * u112m + STORAGE_REAL(FP2R));
	U14m = -0.5 *
		(l3 * y[4] + l4 * u12m - a * u112r + STORAGE_REAL(FP2M));

	/* U122 */
	u122r = (1.0 / d) *
		(-(2.0 * l2 - a * l3) * u12m - 2.0 * y[3] -
		 l1 * y[4] - a * l4 * u112r - l4 * U14m -
		 a * STORAGE_REAL(V12R));
	u122m = (1.0 / d) *
		((2.0 * l2 - a * l3) * u12r + 2.0 * y[2] -
		 l1 * y[5] - a * l4 * u112m + l4 * U14r -
		 a * STORAGE_REAL(V12M));

	/* U24 */
	U24r = -0.5 *
		(l3 * u12r + l4 * u112m + a * u122m + STORAGE_REAL(V12M));
	U24m = -0.5 *
		(l3 * u12m - l4 * u112r - a * u122r - STORAGE_REAL(V12R));

	/* U1122 */
	u1122r = (1.0 / d) *
		(-(2.0 * l2 - a * l3) * u112m - 3.0 * y[4] -
		 l1 * u12r + a * l4 * u122m + l4 * U24m -
		 a * STORAGE_REAL(V22R));
	u1122m = (1.0 / d) *
		((2.0 * l2 - a * l3) * u112r - 3.0 * y[5] -
		 l1 * u12m - a * l4 * u122r - l4 * U24r -
		 a * STORAGE_REAL(V22M));

	U04r = u13m;
	U04m = -u13r;

	U0Ar = U14m;
	U0Am = -U14r;

	U1Ar = 0.5 *
		(-l3 * u12r - l4 * u112m - a * u122m + STORAGE_REAL(V13R));
	U1Am = 0.5 *
		(-l3 * u12m + l4 * u112r + a * u122r + STORAGE_REAL(V13M));

	U2Ar = 0.5 *
		(-l3 * u112m - l4 * u122m + a * u1122r + STORAGE_REAL(V22R));
	U2Am = 0.5 *
		(l3 * u112r + l4 * u122r + a * u1122m + STORAGE_REAL(V22M));

	f[0] = PARAM(LAMBDA_4) * U04r + PARAM(ALPHA) * U0Ar;
	f[1] = PARAM(LAMBDA_4) * U04m + PARAM(ALPHA) * U0Am;
	f[2] = PARAM(LAMBDA_4) * U14r + PARAM(ALPHA) * U1Ar;
	f[3] = PARAM(LAMBDA_4) * U14m + PARAM(ALPHA) * U1Am;
	f[4] = PARAM(LAMBDA_4) * U24r + PARAM(ALPHA) * U2Ar;
	f[5] = PARAM(LAMBDA_4) * U24m + PARAM(ALPHA) * U2Am;
}

static void calculate(catastrophe_t *const catastrophe,
		const unsigned  int i, const unsigned int j)
{
	equation_t *equation;
	point_array_t *point_array;

	assert(catastrophe);

	const double sqrt_pi = sqrt(M_PI);

	/* Gamma function's precalculated values */
	const double g13 = 2.6789385347;
	const double g23 = 1.3541179394;
	const double g14 = 3.625609908;
	const double g34 = 1.225416702;

	/* Precalculate some trigonometric functions */
	const double cs1 = cos(PARAM(K_1) * M_PI / 4.0);
	const double sn1 = sin(PARAM(K_1) * M_PI / 4.0);
	const double c8  = cos(PARAM(K_2) * M_PI / 8.0);
	const double s8  = sin(PARAM(K_2) * M_PI / 8.0);
	const double s38 = sin(PARAM(K_2) * 3.0 * M_PI / 8.0);
	const double c38 = cos(PARAM(K_2) * 3.0 * M_PI / 8.0);

	double FL1r, FL1m;

	equation = catastrophe->equation;
	point_array = catastrophe->point_array;

	assert(equation);
	assert(point_array);

	equation->initial_vector[0] = 0.5 * sqrt_pi * cs1;
	equation->initial_vector[1] = 0.5 * sqrt_pi * sn1;
	equation_set_function(equation, function_1);

	runge_kutta(0.0, 1.0, 0.01, catastrophe);

	FL1r = equation->resulting_vector[0];
	FL1m = equation->resulting_vector[1];

	equation->initial_vector[0] = 0.5 * g14 * c8;
	equation->initial_vector[1] = 0.5 * g14 * s8;
	equation->initial_vector[2] = 0.0;
	equation->initial_vector[3] = 0.0;
	equation->initial_vector[4] = -0.5 * g34 * s38;
	equation->initial_vector[5] =  0.5 * g34 * c38;
	equation_set_function(equation, function_2);

	runge_kutta(0.0, 1.0, 0.01, catastrophe);

	STORAGE_REAL(FP0R) = equation->resulting_vector[0];
	STORAGE_REAL(FP0M) = equation->resulting_vector[1];
	STORAGE_REAL(FP1R) = equation->resulting_vector[2];
	STORAGE_REAL(FP1M) = equation->resulting_vector[3];
	STORAGE_REAL(FP2R) = equation->resulting_vector[4];
	STORAGE_REAL(FP2M) = equation->resulting_vector[5];

	STORAGE_REAL(V12R) = 0.25 *
		(PARAM(LAMBDA_1) * STORAGE_REAL(FP0R) +
		 2.0 * PARAM(LAMBDA_2) * STORAGE_REAL(FP1M));
	STORAGE_REAL(V12M) = 0.25 *
		(PARAM(LAMBDA_1) * STORAGE_REAL(FP0M) -
		 2.0 * PARAM(LAMBDA_2) * STORAGE_REAL(FP1R));
	STORAGE_REAL(V13R) = -STORAGE_REAL(V12M);
	STORAGE_REAL(V13M) = STORAGE_REAL(V12R);
	STORAGE_REAL(V22R) = 0.25 *
		(STORAGE_REAL(FP0R) + PARAM(LAMBDA_1) * STORAGE_REAL(FP1R) +
		 2.0 * PARAM(LAMBDA_2) * STORAGE_REAL(FP2R));
	STORAGE_REAL(V22M) = 0.25 *
		(STORAGE_REAL(FP0M) + PARAM(LAMBDA_1) * STORAGE_REAL(FP1M) +
		 2.0 * PARAM(LAMBDA_2) * STORAGE_REAL(FP2M));

	equation->initial_vector[0] = FL1r * STORAGE_REAL(FP0R) -
		FL1m * STORAGE_REAL(FP0M);
	equation->initial_vector[1] = FL1r * STORAGE_REAL(FP0M) +
		FL1m * STORAGE_REAL(FP0R);
	equation->initial_vector[2] = FL1r * STORAGE_REAL(FP1R) -
		FL1m * STORAGE_REAL(FP1M);
	equation->initial_vector[3] = FL1r * STORAGE_REAL(FP1M) +
		FL1m * STORAGE_REAL(FP1R);
	equation->initial_vector[4] = FL1r * STORAGE_REAL(FP2R) -
		FL1m * STORAGE_REAL(FP2M);
	equation->initial_vector[5] = FL1r * STORAGE_REAL(FP2M) +
		FL1m * STORAGE_REAL(FP2R);
	equation_set_function(equation, catastrophe_Ksub4_2_function);

	runge_kutta(0.0, 1.0, 0.001, catastrophe);
}

static catastrophe_desc_t catastrophe_Ksub4_2_desc = {
	.type = CT_REAL,
	.sym_name = "Ksub4_2",
	.fabric = catastrophe_fabric,
	.num_parameters = 7,
	.par_names = par_names,
	.equation.real = catastrophe_Ksub4_2_function,
	.num_equations = 6,
	.calculate = calculate
};

static int catastrophe_Ksub4_2_init(void) __attribute__ ((constructor));
static int catastrophe_Ksub4_2_init(void)
{
	fprintf(stderr, "Catastrophe Ksub4_2 initialization.\n");
	register_catastrophe_desc(&catastrophe_Ksub4_2_desc);
}
