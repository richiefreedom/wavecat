#include <wavecat/catastrophe.h>
#include <wavecat/point_array.h>
#include <lib/integration/runge_kutta.h>

#include <math.h>

enum indexes {
	VR = 0,
	VM
};

enum parameters {
	LAMBDA_1 = 0,
	LAMBDA_2,
	LAMBDA_3
};

enum variables {
	VARIABLE_K = 0
};

enum storage_entries {
	VA3R,
	VA3M,
	VA3_01R,
	VA3_01M,
	VA3_02R,
	VA3_02M
};

static char *par_names[] = {"l1", "l2", "l3", NULL};
static char *var_names[] = {"k", NULL};

void catastrophe_Csub4_function(const catastrophe_t *const catastrophe,
		const double t, const double *y, double *const f)
{
	double va3r, va3m, va3_01r, va3_01m, va3_02r, va3_02m;
	double U01r, U01m, U02r, U02m, U03r, U03m, U12r, U12m;
	double param1, param2, param3;
	double l1, l2, l3;
	double k;

	param1 = catastrophe->parameter[LAMBDA_1].cur_value;
	param2 = catastrophe->parameter[LAMBDA_2].cur_value;
	param3 = catastrophe->parameter[LAMBDA_3].cur_value;

	k = catastrophe->variable[VARIABLE_K].cur_value;
	va3r = STORAGE_REAL(VA3R);
	va3m = STORAGE_REAL(VA3M);
	va3_01r = STORAGE_REAL(VA3_01R);
	va3_01m = STORAGE_REAL(VA3_01M);
	va3_02r = STORAGE_REAL(VA3_01R);
	va3_02m = STORAGE_REAL(VA3_01M);

	l1 = param1 * t;
	l2 = param2 * t;
	l3 = param3 * t;

	U01r = -va3r + l3 * y[VM];
	U01m = -va3m - l3 * y[VR];

	U02r = -va3_01m - l3 * U01r;
	U02m = va3_01r - l3 * U01m;

	U12r = -va3_02r + l3 * U02m;
	U12m = -va3_02m - l3 * U02r;

	U03r = -4.0 * k * U12m - 2.0 * l2 * U01r + l1 * y[VM];
	U03m = 4.0 * k * U12r - 2.0 * l2 * U01m - l1 * y[VR];

	f[VR] = param1 * U01r + param2 * U02r + param3 * U03r;
	f[VM] = param1 * U01m + param2 * U02m + param3 * U03m;
}

extern void catastrophe_Asub3_function(const catastrophe_t *const catastrophe,
		const double t, const double *y, double *const f);

static void calculate(catastrophe_t *const catastrophe,
		const unsigned  int i, const unsigned int j)
{
	equation_t *equation;
	point_array_t *point_array;

	assert(catastrophe);

	const double k1 = 1.0, k2 = 1.0;

	/* Gamma function's precalculated values */
	const double g14 = 3.625609908;
	const double g34 = 1.225416702;

	/* Precalculate some trigonometric functions */
	double c8  = cos(k2 * M_PI / 8.0);
	double s8  = sin(k2 * M_PI / 8.0);
	double s38 = sin(k2 * 3.0 * M_PI / 8.0);
	double c38 = cos(k2 * 3.0 * M_PI / 8.0);

	double module;
	double phase;

	equation = catastrophe->equation;
	point_array = catastrophe->point_array;

	assert(equation);
	assert(point_array);

	equation->initial_vector[0] = 0.5 * g14 * c8;
	equation->initial_vector[1] = 0.5 * g14 * s8;
	equation->initial_vector[2] = 0.0;
	equation->initial_vector[3] = 0.0;
	equation->initial_vector[4] = -0.5 * g34 * s38;
	equation->initial_vector[5] =  0.5 * g34 * c38;
	equation_set_function(equation, catastrophe_Asub3_function);

	runge_kutta(0.0, 1.0, 0.001, catastrophe);

	STORAGE_REAL(VA3R) = equation->resulting_vector[0];
	STORAGE_REAL(VA3M) = equation->resulting_vector[1];
	STORAGE_REAL(VA3_01R) = equation->resulting_vector[2];
	STORAGE_REAL(VA3_01M) = equation->resulting_vector[3];
	STORAGE_REAL(VA3_02R) = equation->resulting_vector[4];
	STORAGE_REAL(VA3_02M) = equation->resulting_vector[5];

	equation->initial_vector[0] = M_PI;
	equation->initial_vector[1] = 0;
	equation_set_function(equation, catastrophe_Csub4_function);

	runge_kutta(0.0, 1.0, 0.001, catastrophe);

	module = sqrt(equation->resulting_vector[0] *
			equation->resulting_vector[0] +
			equation->resulting_vector[1] *
			equation->resulting_vector[1]);
	phase = (180.0 / M_PI) * atan2(equation->resulting_vector[0],
			equation->resulting_vector[1]);

	point_array->array[i][j].module = module;
	point_array->array[i][j].phase = phase;
}

static catastrophe_desc_t catastrophe_Csub4_desc = {
	.type = CT_REAL,
	.sym_name = "Csub4",
	.fabric = catastrophe_fabric,
	.num_parameters = 3,
	.num_variables = 7,
	.par_names = par_names,
	.var_names = var_names,
	.equation.real = catastrophe_Csub4_function,
	.num_equations = 6,
	.calculate = calculate
};

static int catastrophe_Csub4_init(void) __attribute__ ((constructor));
static int catastrophe_Csub4_init(void)
{
	fprintf(stderr, "Catastrophe Csub4 initialization.\n");
	register_catastrophe_desc(&catastrophe_Csub4_desc);
}
