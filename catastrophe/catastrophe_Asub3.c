#include <wavecat/catastrophe.h>
#include <wavecat/point_array.h>
#include <lib/integration/runge_kutta.h>

#include <math.h>

enum parameters {
	LAMBDA_1 = 0,
	LAMBDA_2
};

static char *par_names[] = {"l1", "l2", NULL};

void catastrophe_Asub3_function(const catastrophe_t *const catastrophe,
		const double t, const double *y, double *const f)
{
	double l1, l2, i21m, i21r, i22m, i22r;
	double param1, param2;

	param1 = catastrophe->parameter[LAMBDA_1].cur_value;
	param2 = catastrophe->parameter[LAMBDA_2].cur_value;

	l1 = param1 * t;
	l2 = param2 * t;

	i21r = 0.25 * (l1 * y[0] + 2.0 * l2 * y[3]);
	i21m = 0.25 * (l1 * y[1] - 2.0 * l2 * y[2]);
	i22r = 0.25 * (y[1] + l1 * y[3] + 2.0 * l2 * y[5]);
	i22m = 0.25 * (-y[0] - l1 * y[2] - 2.0 * l2 * y[4]);

	f[0] = param1 * y[2] + param2 * y[4];
	f[1] = param1 * y[3] + param2 * y[5];

	f[2] = -param1 * y[5] + param2 * i21r;
	f[3] = param1 * y[4] + param2 * i21m;

	f[4] = param1 * i21r + param2 * i22r;
	f[5] = param1 * i21m + param2 * i22m;
}

static void calculate(catastrophe_t *const catastrophe,
		const unsigned  int i, const unsigned int j)
{
	equation_t *equation;
	point_array_t *point_array;

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

	assert(catastrophe);

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

static catastrophe_desc_t catastrophe_Asub3_desc = {
	.type = CT_REAL,
	.sym_name = "Asub3",
	.fabric = catastrophe_fabric,
	.num_parameters = 2,
	.num_variables = 0,
	.par_names = par_names,
	.var_names = NULL,
	.equation.real = catastrophe_Asub3_function,
	.num_equations = 6,
	.calculate = calculate
};

static int catastrophe_Asub3_init(void) __attribute__ ((constructor));
static int catastrophe_Asub3_init(void)
{
	fprintf(stderr, "Catastrophe Asub3 initialization.\n");
	register_catastrophe_desc(&catastrophe_Asub3_desc);
}
