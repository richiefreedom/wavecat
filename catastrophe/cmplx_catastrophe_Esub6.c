#include <kernel/core/point_array.h>
#include <kernel/integration/cmplx_runge_kutta.h>

#include <math.h>

/*
 * General catastrophe Esub6. W = (V, V1, V2, V3, V4, V5), gamma = 6.
 */

enum parameters {
	LAMBDA_1 = 0,
	LAMBDA_2,
	LAMBDA_3,
	LAMBDA_4,
	LAMBDA_5,
	K_1,
	K_2
};

enum components {
	V = 0,
	V1,
	V2,
	V3,
	V4,
	V5
};

static char *par_names[] = {"l1", "l2", "l3", "l4", "l5", "k1", "k2", NULL};

void cmplx_catastrophe_Esub6_function(
		const catastrophe_t *const catastrophe,
		const double t, const double complex *y,
		double complex *const f)
{
	double l1, l2, l3, l4, l5;
	double complex
		U31, U22, U11,
		U32, U41, U21,  U42,
		U33, U51, U43,  U44,
		U52, U54, U222, U422,
		U53, U55, U333, U331;

	l1 = PARAM(LAMBDA_1) * t;
	l2 = PARAM(LAMBDA_2) * t;
	l3 = PARAM(LAMBDA_3) * t;
	l4 = PARAM(LAMBDA_4) * t;
	l5 = PARAM(LAMBDA_5) * t;

	/* Depends on nothing */
	U31 = PARAM(K_1) / 4.0 *
		(l1 * y[V] - 2.0 * I * l3 * y[V1] - I * l4 * y[V2] -
		 2.0 * I * l5 * y[V4]);

	/* Depends on nothing */
	U22 = PARAM(K_2) / 3.0 *
		(l2 * y[V] - I * l5 * y[V3] - I * l4 * y[V1]);

	U11 = I * y[V3];
	U32 = I * y[V5];
	U41 = I * y[V5];
	U21 = I * y[V4];

	/* Depends on U31 */
	U42 = -I * PARAM(K_2) / 3.0 *
		(l2 * y[V1] - I * l5 * U31 + l4 * y[V3]);

	/* Depends on nothing */
	U33 = -I * PARAM(K_1) / 4.0 *
		(y[V] + l1 * y[V1] + 2.0 * l3 * y[V3] + l4 * y[V4] + 2.0 *
		 l5 * y[V5]);

	/* Depends on U22, U42 */
	U51 = U43 = I * PARAM(K_1) / 4.0 *
		(I * l4 * U22 + 2.0 * I * l5 * U42 - l1 * y[V2] -
		 2.0 * l3 * y[V4]);

	/* Depends on U33, U31 */
	U44 = U52 = I * PARAM(K_2) / 3.0 *
		(I * l5 * U33 + I * l4 * U31 - l2 * y[V3]);

	/* Depends on nothing */
	U222 = PARAM(K_2) / 3.0 *
		(y[V] + l2 * y[V2] + l5 * y[V5] + l4 * y[V4]);

	/* Depends on U51 */
	U422 = -I * PARAM(K_2) / 3.0 *
		(y[V1] + I * l2 * y[V4] + I * l4 * y[V5] + l5 * U51);

	/* Depends on U22, U42, U222, U422 */
	U54 = -PARAM(K_1) / 4.0 *
		(l1 * U22 + 2.0 * l3 * U42 - I * l4 * U222 - 2.0 * I * l5 *
		 U422);

	/* Depends on U42, U52 */
	U53 = -PARAM(K_1) / 4.0 *
		(y[V2] + I * l1 * y[V4] + 2.0 * I * l3 * y[V5] + l4 * U42 +
		 2.0 * l5 * U52);

	/* Depends on U31, U33, U51, U53 */
	U333 = -I * PARAM(K_1) / 4.0 *
		(3.0 * y[V3] + l1 * U31 + 2.0 * l3 * U33 + l4 * U51 + 2.0 *
		 l5 * U53);

	/* Depends on U31, U43 */
	U331 = PARAM(K_1) / 4.0 *
		(l1 * y[V3] - 2.0 * I * l3 * U31 - 2.0 * I * y[V1] + l4 *
		 y[V5] - 2.0 * I * l5 * U43);

	/* Depends on U33, U333, U331 */
	U55 = -PARAM(K_2) / 3.0 *
		(l2 * U33 - I * l5 * U333 - I * l4 * U331);

	f[V] =  y[V1] * PARAM(LAMBDA_1) +
		y[V2] * PARAM(LAMBDA_2) +
		y[V3] * PARAM(LAMBDA_3) +
		y[V4] * PARAM(LAMBDA_4) +
		y[V5] * PARAM(LAMBDA_5);

	f[V1] = U11 * PARAM(LAMBDA_1) +
		U21 * PARAM(LAMBDA_2) +
		U31 * PARAM(LAMBDA_3) +
		U41 * PARAM(LAMBDA_4) +
		U51 * PARAM(LAMBDA_5);

	f[V2] = U21 * PARAM(LAMBDA_1) +
		U22 * PARAM(LAMBDA_2) +
		U32 * PARAM(LAMBDA_3) +
		U42 * PARAM(LAMBDA_4) +
		U52 * PARAM(LAMBDA_5);

	f[V3] = U31 * PARAM(LAMBDA_1) +
		U32 * PARAM(LAMBDA_2) +
		U33 * PARAM(LAMBDA_3) +
		U43 * PARAM(LAMBDA_4) +
		U53 * PARAM(LAMBDA_5);

	f[V4] = U41 * PARAM(LAMBDA_1) +
		U42 * PARAM(LAMBDA_2) +
		U43 * PARAM(LAMBDA_3) +
		U44 * PARAM(LAMBDA_4) +
		U54 * PARAM(LAMBDA_5);

	f[V5] = U51 * PARAM(LAMBDA_1) +
		U52 * PARAM(LAMBDA_2) +
		U53 * PARAM(LAMBDA_3) +
		U54 * PARAM(LAMBDA_4) +
		U55 * PARAM(LAMBDA_5);
}

static void calculate(catastrophe_t *const catastrophe,
		const unsigned int i, const unsigned int j)
{
	cmplx_equation_t *equation;
	point_array_t *point_array;

	/* Gamma function's precalculated values */
	static const double g14 = 3.625609908;
	static const double g34 = 1.225416702;
	static const double g13 = 2.678938534;
	static const double g23 = 1.354117939;

	double module;
	double phase;

	assert(catastrophe);

	equation = catastrophe->equation;
	point_array = catastrophe->point_array;

	assert(equation);
	assert(point_array);

	equation->initial_vector[V] = 0.5 / sqrt(3.0) * g13 * g14 *
		cexp(I * PARAM(K_1) * M_PI / 8.0);

	equation->initial_vector[V1] = 0;

	equation->initial_vector[V2] = -PARAM(K_2) *
		(1.0 / (2.0 * sqrt(3.0))) * g23 * g14 *
		cexp(I * PARAM(K_1) * M_PI / 8.0);

	equation->initial_vector[V3] = (I / (2.0 * sqrt(3.0))) * g13 * g34 *
		cexp(I * PARAM(K_1) * 3.0 * M_PI / 8.0);

	equation->initial_vector[V4] = 0;

	equation->initial_vector[V5] = -PARAM(K_2) / (2.0 * sqrt(3.0)) *
		g23 * g34 * cexp(I * PARAM(K_1) * 3.0 * M_PI / 8.0);

	cmplx_runge_kutta(0.0, 1.0, 0.01, catastrophe);

	module = cabs(equation->resulting_vector[V]);
	phase = (180.0 / M_PI) * carg(equation->resulting_vector[V]);

	point_array->array[i][j].module = module;
	point_array->array[i][j].phase = phase;
}

static catastrophe_desc_t cmplx_catastrophe_Esub6_desc = {
	.type = CT_COMPLEX,
	.sym_name = "Esub6",
	.fabric = catastrophe_fabric,
	.num_parameters = 7,
	.par_names = par_names,
	.equation.cmplx = cmplx_catastrophe_Esub6_function,
	.num_equations = 6,
	.calculate = calculate
};

static int cmplx_catastrophe_Esub6_init(void) __attribute__ ((constructor));
static int cmplx_catastrophe_Esub6_init(void)
{
	fprintf(stderr, "Catastrophe Esub6 (complex) initialization.\n");
	register_catastrophe_desc(&cmplx_catastrophe_Esub6_desc);
}
