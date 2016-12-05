#include <kernel/core/point_array.h>
#include <kernel/integration/cmplx_runge_kutta.h>

#include <math.h>

/*
 * General catastrophe Fsub1_0. W = (V, V1, V2, V3), gamma = 4.
 */

enum parameters {
	LAMBDA_1 = 0,
	LAMBDA_2,
	LAMBDA_3,
	LAMBDA_4,
	ALPHA
};

enum storage_entries {
	Ai = 0,
	Aid
};

enum components {
	V = 0,
	V1,
	V2,
	V3
};

static char *par_names[] = {"l1", "l2", "l3", "l4", "a", NULL};

void cmplx_catastrophe_Fsub1_0_function(
		const catastrophe_t *const catastrophe,
		const double t, const double complex *y,
		double complex *const f)
{
	double l1, l2, l3, l4, a, r13;
	double complex ri3;
	double complex U22, U21, U12, U01, U02, U03, U04, U11, U31, U13, U24,
	       U23, U32, U05, U14, U34, U33, U15, U25, U133, U114, U134, U35;

	l1 = PARAM(LAMBDA_1) * t;
	l2 = PARAM(LAMBDA_2) * t;
	l3 = PARAM(LAMBDA_3) * t;
	l4 = PARAM(LAMBDA_4) * t;
	a = PARAM(ALPHA) * t;

	r13 = 1.0 / 3.0;
	ri3 = I / 3.0;

	U22 = r13 * (l2 * y[V] - I * l3 * y[V1] - 2.0 * I * a * y[V3]);
	U21 = U12 = I * y[V3];
	U01 = y[V1];
	U02 = y[V2];
	U03 = y[V3];
	U04 = -ri3 *
		(l1 * y[V] - 2.0 * I * l4 * y[V1] - I * l3 * y[V2] - a * U22 -
		 I * STORAGE_COMPLEX(Ai));
	U11 = I * U04;
	U31 = U13 = U24 = -1.0 / (4.0 * pow(a, 3) + 27.0) *
		(-3.0 * (I * l2 * l3 + a) * y[V] +
		 (2.0 * a * a * l2 - 3.0 * l3 * l3) * y[V1] +
		 (9.0 * l1 - 3.0 * a * l2) * y[V2] +
		 (18.0 * l4 - 9.0 * a * l3) * y[V3] + 2.0 * a * a * l3 * U04 -
		 9.0 * I * STORAGE_COMPLEX(Aid));
	U23 = U32 = -ri3 *
		(l2 * y[V1] + l3 * U04 - 2.0 * I * a * U13);
	U05 = -I * U32;
	U14 = -ri3 *
		(l1 * y[V1] + y[V] + 2.0 * l4 * U04 - I * l3 * U12 -
		 I * a * U32);
	U34 = -1.0 / (4.0 * pow(a, 3) + 27.0) *
		(-3.0 * (l2 * l3 * I + a) * y[V1] +
		 I * (2.0 * a * a * l2 - 3.0 * l3 * l3) * U04 +
		 I * (9.0 * l1 - 3.0 * a * l2) * y[V3] + 9.0 * y[V2] +
		 (18.0 * l4 - 9.0 * a * l3) * U13 + 2.0 * a * a * l3 * U14);
	U33 = U15 = -r13 *
		(l2 * U11 + l3 * U14 + 2.0 * a * U34);
	U25 = -ri3 *
		(l2 * y[V3] + l3 * U13 - 2.0 * I * a * U33);
	U114 = -ri3 *
		(2.0 * y[V1] + I * l1 * U04 + 2.0 * l4 * U14 +
		 l3 * U13 + a * U33);
	U134 = -1.0 / (4.0 * pow(a, 3) + 27.0) *
		(-3.0 * (I * l2 * l3 + a) * U11 +
		 I * (2.0 * a * a * l2 - 3.0 * l3 * l3) * U14 +
		 9.0 * I * y[V3] + I * (9.0 * l1 - 3.0 * a * l2) * U31 +
		 9.0 * U05 + 2.0 * a * a * l3 * U114 +
		 I * (18.0 * l4 - 9.0 * a * l3) * U34);
	U133 = -r13 *
		(I * l2 * U14 + l3 * U114 + 2.0 * a * U134);
	U35 = ri3 *
		(I * l2 * U13 + U11 + I * l3 * U34 + 2.0 * a * U133);


	f[V] =  PARAM(LAMBDA_1) * U01 +
		PARAM(LAMBDA_2) * U02 +
		PARAM(LAMBDA_3) * U03 +
		PARAM(LAMBDA_4) * U04 +
		PARAM(ALPHA) * U05;

	f[V1] = PARAM(LAMBDA_1) * U11 +
		PARAM(LAMBDA_2) * U12 +
		PARAM(LAMBDA_3) * U13 +
		PARAM(LAMBDA_4) * U14 +
		PARAM(ALPHA) * U15;

	f[V2] = PARAM(LAMBDA_1) * U21 +
		PARAM(LAMBDA_2) * U22 +
		PARAM(LAMBDA_3) * U23 +
		PARAM(LAMBDA_4) * U24 +
		PARAM(ALPHA) * U25;

	f[V3] = PARAM(LAMBDA_1) * U31 +
		PARAM(LAMBDA_2) * U32 +
		PARAM(LAMBDA_3) * U33 +
		PARAM(LAMBDA_4) * U34 +
		PARAM(ALPHA) * U35;
}

static void cmplx_catastrophe_Airy_function(
		const catastrophe_t *const catastrophe,
		const double t, const double complex *y,
		double complex *const f)
{
	double l1;
	double complex U11;

	l1 = PARAM(LAMBDA_2) * t;

	U11 = l1 / 3.0 * y[V];

	f[V]  = PARAM(LAMBDA_2) * y[V1];
	f[V1] = PARAM(LAMBDA_2) * U11;
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

	static double r13sqr3;
	static double complex ri3sqrt3;

	double module;
	double phase;

	double divsqrt3 = 1.0 / sqrt(3.0);

	assert(catastrophe);

	equation = catastrophe->equation;
	point_array = catastrophe->point_array;

	assert(equation);
	assert(point_array);

	r13sqr3 = 1.0 / (3.0 * sqrt(3.0));
	ri3sqrt3 = I * r13sqr3;

	equation->initial_vector[V] = divsqrt3 * g13;
	equation->initial_vector[V1] = -divsqrt3 * g23;
	equation_set_function(equation, cmplx_catastrophe_Airy_function);
	cmplx_runge_kutta(0.0, 1.0, 0.01, catastrophe);
	STORAGE_COMPLEX(Ai)  = equation->resulting_vector[V];
	STORAGE_COMPLEX(Aid) = equation->resulting_vector[V1];

	equation->initial_vector[V] = r13sqr3 * pow(g13, 2) *
		cexp(I * M_PI / 6.0);
	equation->initial_vector[V1] = ri3sqrt3 * g13 * g23 *
		cexp(I * M_PI / 3.0);
	equation->initial_vector[V2] = -r13sqr3 * g13 * g23 *
		cexp(I * M_PI / 6.0);
	equation->initial_vector[V3] = -ri3sqrt3 * pow(g23, 2) *
		cexp(I * M_PI / 3.0);

	equation_set_function(equation, cmplx_catastrophe_Fsub1_0_function);

	cmplx_runge_kutta(0.0, 1.0, 0.001, catastrophe);

	module = cabs(equation->resulting_vector[V]);
	phase = (180.0 / M_PI) * carg(equation->resulting_vector[V]);

	point_array->array[i][j].module = module;
	point_array->array[i][j].phase = phase;
}

static catastrophe_desc_t cmplx_catastrophe_Fsub1_0_desc = {
	.type = CT_COMPLEX,
	.sym_name = "Fsub1_0",
	.fabric = catastrophe_fabric,
	.num_parameters = 5,
	.par_names = par_names,
	.equation.cmplx = cmplx_catastrophe_Fsub1_0_function,
	.num_equations = 4,
	.calculate = calculate
};

static int cmplx_catastrophe_Fsub_1_0_init(void) __attribute__ ((constructor));
static int cmplx_catastrophe_Fsub_1_0_init(void)
{
	fprintf(stderr, "Catastrophe Fsub1_0 (complex) initialization.\n");
	register_catastrophe_desc(&cmplx_catastrophe_Fsub1_0_desc);
}
