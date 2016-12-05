#include <kernel/core/point_array.h>
#include <kernel/integration/cmplx_runge_kutta.h>

#include <math.h>

/*
 * General catastrophe Psub8. W = (V, V1, V2, V3, V4, V5, V6, Va), gamma = 8.
 */

enum parameters {
	LAMBDA_1 = 0,
	LAMBDA_2,
	LAMBDA_3,
	LAMBDA_4,
	LAMBDA_5,
	LAMBDA_6,
	ALPHA
};

enum components {
	V = 0,
	V1,
	V2,
	V3,
	V4,
	V5,
	V6,
	Va
};

static char *par_names[] = {"l1", "l2", "l3", "l4", "l5", "l6", "a", NULL};

void cmplx_catastrophe_Psub8_function(
		const catastrophe_t *const catastrophe,
		const double t, const double complex *y,
		double complex *const f)
{
	double l1, l2, l3, l4, l5, l6, a, r13, r9a27;
	double complex ri3;
	double complex U25, U16, U34, U11, U22, U33, U14, U35, U26, U24, U36,
	       U15, U222, U44, U333, U45, U1a, U54, U46, U64, U2a, U111, U114,
	       U55, U56, U65, U3a, U224, U66, U226, U336, U4a, Ua4, U335,
	       U3333, U5a, Ua5, U2222, U244, U6a, Ua6, U144, U246, U2224,
	       U444, Uaa;

	l1 = PARAM(LAMBDA_1);
	l2 = PARAM(LAMBDA_2);
	l3 = PARAM(LAMBDA_3);
	l4 = PARAM(LAMBDA_4) * t;
	l5 = PARAM(LAMBDA_5) * t;
	l6 = PARAM(LAMBDA_6) * t;
	a = PARAM(ALPHA) * t;

	r13 = 1.0 / 3.0;
	ri3 = I / 3.0;
	r9a27 = 9.0 / (pow(a, 3) + 27.0);

	U25 = U16 = U34 = I * y[Va];

	U11 = r13 * (l1 * y[V] - I * a * y[V6] - I * l4 * y[V2] -
			I * l5 * y[V3]);

	U22 = r13 * (l2 * y[V] - I * a * y[V5] - I * l4 * y[V1] -
			I * l6 * y[V3]);

	U33 = r13 * (l3 * y[V] - I * a * y[V4] - I * l5 * y[V1] -
			I * l6 * y[V2]);

	U14 = -I * r9a27 *
		(l1 * y[V2] + (a * a / 9.0) * (l3 * y[V1] + l6 * y[V4]) +
		 l5 * y[V6] - (a / 3.0) * (l2 * y[V3] + l4 * y[V5]) -
		 I * l4 * U22 + I / 3.0 * a * l6 * U33 -
		 I / 9.0 * l5 * a * a * U11);

	U35 = -ri3 * (l3 * y[V1] + l6 * y[V4] - I * l5 * U11 - I * a * U14);

	U26 = -ri3 * (l2 * y[V3] + l4 * y[V5] - I * l6 * U33 - I * a * U35);

	U24 = -I * r9a27 *
		(l2 * y[V1] + l6 * y[V5] -
		 a / 3.0 * (l1 * y[V3] + l4 * y[V6]) +
		 a * a / 9.0 * (l3 * y[V2] + l5 * y[V4]) -
		 I * l4 * U11 +
		 I / 3.0 * a * l5 * U33 -
		 I / 9.0 * a * a * l6 * U22);

	U36 = -ri3 * (l3 * y[V2] + l5 * y[V4] - I * l6 * U22 - I * a * U24);

	U15 = -ri3 * (l1 * y[V3] + l4 * y[V6] - I * l5 * U33 - I * a * U36);

	U222 = r13 * (y[V] + l2 * y[V2] + l4 * y[V4] + l6 * y[V6] + a * y[Va]);

	U44 = -r9a27 *
		(l1 * U22 + a * a / 9.0 * (I * l3 * y[V4] + l6 * U24) +
		 l5 * U26 -
		 a / 3.0 * (y[V3] + I * l2 * y[V6] + I * l4 * y[Va]) -
		 I * l4 * U222 - a * l6 / 3.0 * U35 + l5 * a * a / 9.0 * U14);

	U333 = r13 *
		(y[V] + l3 * y[V3] + l5 * y[V5] + l6 * y[V6] + a * y[Va]);

	U45 = U1a = U54 = -r9a27 *
		(I * l1 * y[V6] +
		 a * a / 9.0 * (y[V1] + I * l3 * y[V5] + I * l6 * y[Va]) +
		 l5 * U36 - a / 3.0 * (l2 * U33 + l4 * U35) + l4 * U26 +
		 I * a * l6 / 3.0 * U333 + l5 * a * a / 9.0 * U15);

	U46 = U64 = U2a = -r9a27 *
		(I * l2 * y[V5] + l6 * U35 - a / 3.0 * (l1 * U33 + l4 * U36) +
		 a * a / 9.0 * (y[V2] + I * l3 * y[V6] + I * l5 * y[Va]) +
		 l4 * U15 + I * a * l5 / 3.0 * U333 + a * a / 9.0 * l6 * U26);

	U111 = r13 *
		(y[V] + l1 * y[V1] + l4 * y[V4] + l5 * y[V5] + a * y[Va]);

	U114 = r13 *
		(l1 * y[V4] - I * a * U46 - I * y[V2] -
		 I * l4 * U24 + l5 * y[Va]);

	U55 = -r13 *
		(l3 * U11 + l6 * U14 - I * l5 * U111 - I * a * U114);

	U56 = U65 = U3a = -r13 *
		(I * l3 * y[V4] + l6 * U24 + l5 * U14 + a * U44);

	U224 = r13 *
		(l2 * y[V4] - I * a * U45 - I * y[V1] -
		 I * l4 * U14 + l6 * y[Va]);

	U66 = -r13 *
		(l3 * U22 + l5 * U24 - I * l6 * U222 - I * a * U224);

	U226 = r13 * (l2 * y[V6] - I * a * U56 + l4 * y[Va] - I * l6 * U36);

	U336 = -ri3 *
		(y[V2] + I * l3 * y[V6] + a * U2a + I * l5 * y[Va] + l6 * U26);

	U4a = Ua4 = I * r9a27 *
		(I * l1 * U26 +
		 a * a / 9.0 * (I * y[V4] - l3 * y[Va] + I * l6 * U2a) +
		 I * l5 * U66 -
		 a / 3.0 * (U33 + I * l2 * U36 + I * l4 * U56) +
		 l4 * U226 - a * l6 / 3.0 * U336 + I * l5 * a * a / 9.0 * U45);

	U335 = r13 *
		(l3 * y[V5] - I * a * U45 - I * y[V1] -
		 I * l5 * U15 + l6 * y[Va]);

	U3333 = r13 *
		(2.0 * y[V3] + l3 * U33 + a * U3a + l5 * U35 + l6 * U36);

	U5a = Ua5 = I * r9a27 *
		(I * l1 * U36 +
		 a * a / 9.0 * (2.0 * I * y[V5] +
			 I * l3 * U35 + I * l6 * U3a) +
		 l5 * U336 - a / 3.0 * (l2 * U333 + l4 * U335) +
		 I * l4 * U66 + I * a * l6 / 3.0 * U3333 +
		 I / 9.0 * l5 * a * a * U55);

	U2222 = r13 *
		(2.0 * y[V2] + l2 * U22 + a * U2a + l4 * U24 + l6 * U26);

	U244 = -r9a27 *
		(l1 * U222 + a * a / 9.0 * (I * l3 * U24 + l6 * U224) +
		 l5 * U226 -
		 a / 3.0 * (2.0 * I * y[V6] + I * l2 * U26 + I * l4 * U2a) -
		 I * l4 * U2222 - I / 3.0 * a * l6 * U66 +
		 I / 9.0 * l5 * a * a * U44);

	U6a = Ua6 = -r13 *
		(l3 * U24 + l5 * U44 - I * l6 * U224 - I * a * U244);

	U144 = -r9a27 *
		(U22 + I * l1 * U24 +
		 a * a / 9.0 * (I * l3 * U14 + I * l6 * U44) +
		 I * l5 * U46 -
		 a / 3.0 * (I * y[V5] - l2 * y[Va] + I * l4 * U1a) +
		 l4 * U224 - I / 3.0 * a * l6 * U56 + a * a / 9.0 * l5 * U114);

	U246 = -r9a27 *
		(I * y[V5] + I * l2 * U25 + I * l6 * U56 -
		 a / 3.0 * (I * l1 * U36 + I * l4 * U66) +
		 a * a / 9.0 * (U22 + I * l3 * U26 + I * l5 * U2a) +
		 I * l4 * U45 - a / 3.0 * l5 * U336 + a * a / 9.0 * l6 * U226);

	U2224 = r13 *
		(2.0 * y[V4] + l2 * U24 + a * Ua4 + l4 * U44 + l6 * U46);

	U444 = -r9a27 *
		(l1 * U224 +
		 a * a / 9.0 * (I * l3 * U44 + l6 * U244 + l5 * U144) -
		 a / 3.0 * (2.0 * I * y[Va] + I * l2 * U46 + I * U4a +
			 I * l6 * Ua6) +
		 l5 * U246 - I * U222 - I * l4 * U2224);

	Uaa = ri3 *
		(I * l3 * U44 + l5 * U144 + l6 * U244 + a * U444);

	f[V] =  PARAM(LAMBDA_4) * y[V4] +
		PARAM(LAMBDA_5) * y[V5] +
		PARAM(LAMBDA_6) * y[V6] +
		PARAM(ALPHA) * y[Va];

	f[V1] = PARAM(LAMBDA_4) * U14 +
		PARAM(LAMBDA_5) * U15 +
		PARAM(LAMBDA_6) * U16 +
		PARAM(ALPHA) * U1a;

	f[V2] = PARAM(LAMBDA_4) * U24 +
		PARAM(LAMBDA_5) * U25 +
		PARAM(LAMBDA_6) * U26 +
		PARAM(ALPHA) * U2a;

	f[V3] = PARAM(LAMBDA_4) * U34 +
		PARAM(LAMBDA_5) * U35 +
		PARAM(LAMBDA_6) * U36 +
		PARAM(ALPHA) * U3a;

	f[V4] = PARAM(LAMBDA_4) * U44 +
		PARAM(LAMBDA_5) * U45 +
		PARAM(LAMBDA_6) * U46 +
		PARAM(ALPHA) * U4a;

	f[V5] = PARAM(LAMBDA_4) * U54 +
		PARAM(LAMBDA_5) * U55 +
		PARAM(LAMBDA_6) * U56 +
		PARAM(ALPHA) * U5a;

	f[V6] = PARAM(LAMBDA_4) * U64 +
		PARAM(LAMBDA_5) * U65 +
		PARAM(LAMBDA_6) * U66 +
		PARAM(ALPHA) * U6a;

	f[Va] = PARAM(ALPHA) * Uaa;
}

static void cmplx_catastrophe_Airyl1_function(
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

static void cmplx_catastrophe_Airyl2_function(
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

static void cmplx_catastrophe_Airyl3_function(
		const catastrophe_t *const catastrophe,
		const double t, const double complex *y,
		double complex *const f)
{
	double l1;
	double complex U11;

	l1 = PARAM(LAMBDA_3) * t;

	U11 = l1 / 3.0 * y[V];

	f[V]  = PARAM(LAMBDA_3) * y[V1];
	f[V1] = PARAM(LAMBDA_3) * U11;
}

static void calculate(catastrophe_t *const catastrophe,
		const unsigned int i, const unsigned int j)
{
	cmplx_equation_t *equation;
	point_array_t *point_array;

	double module;
	double phase;

	double ail1, ail2, ail3,
	       aidl1, aidl2, aidl3;

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
	equation_set_function(equation, cmplx_catastrophe_Airyl1_function);
	cmplx_runge_kutta(0.0, 1.0, 0.01, catastrophe);
	ail1 = equation->resulting_vector[V];
	aidl1 = equation->resulting_vector[V1];

	equation->initial_vector[V] = divsqrt3 * g13;
	equation->initial_vector[V1] = -divsqrt3 * g23;
	equation_set_function(equation, cmplx_catastrophe_Airyl2_function);
	cmplx_runge_kutta(0.0, 1.0, 0.01, catastrophe);
	ail2 = equation->resulting_vector[V];
	aidl2 = equation->resulting_vector[V1];

	equation->initial_vector[V] = divsqrt3 * g13;
	equation->initial_vector[V1] = -divsqrt3 * g23;
	equation_set_function(equation, cmplx_catastrophe_Airyl3_function);
	cmplx_runge_kutta(0.0, 1.0, 0.01, catastrophe);
	ail3 = equation->resulting_vector[V];
	aidl3 = equation->resulting_vector[V1];

	equation->initial_vector[V] = ail1 * ail2 * ail3;
	equation->initial_vector[V1] = aidl1 * ail2 * ail3;
	equation->initial_vector[V2] = ail1 * aidl2 * ail3;
	equation->initial_vector[V3] = ail1 * ail2 * aidl3;
	equation->initial_vector[V4] = -I * aidl1 * aidl2 * ail3;
	equation->initial_vector[V5] = -I * aidl1 * ail2 * aidl3;
	equation->initial_vector[V6] = -I * ail1 * aidl2 * aidl3;
	equation->initial_vector[Va] = -aidl1 * aidl2 * aidl3;

	equation_set_function(equation, cmplx_catastrophe_Psub8_function);

	cmplx_runge_kutta(0.0, 1.0, 0.01, catastrophe);

	module = cabs(equation->resulting_vector[V]);
	phase = (180.0 / M_PI) * carg(equation->resulting_vector[V]);

	point_array->array[i][j].module = module;
	point_array->array[i][j].phase = phase;
}

static catastrophe_desc_t cmplx_catastrophe_Psub8_desc = {
	.type = CT_COMPLEX,
	.sym_name = "Psub8",
	.fabric = catastrophe_fabric,
	.num_parameters = 7,
	.par_names = par_names,
	.equation.cmplx = cmplx_catastrophe_Psub8_function,
	.num_equations = 8,
	.calculate = calculate
};

static int cmplx_catastrophe_Psub8_init(void) __attribute__ ((constructor));
static int cmplx_catastrophe_Psub8_init(void)
{
	fprintf(stderr, "Catastrophe Psub8 (complex) initialization.\n");
	register_catastrophe_desc(&cmplx_catastrophe_Psub8_desc);
}
