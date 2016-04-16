#ifndef _WAVECAT_POINT_ARRAY_H_
#define _WAVECAT_POINT_ARRAY_H_

#include <kernel/core/config.h>
#include <stdlib.h>

extern FILE *out_file_desc;

struct point_s {
	double module;
	double phase;
};

typedef struct point_s point_t;

struct point_array_s {
	double min_x;
	double max_x;
	unsigned int num_steps_x;
	double min_y;
	double max_y;
	unsigned int num_steps_y;
	point_t **array;
};

typedef struct point_array_s point_array_t;

static inline point_array_t *construct_point_array(
		double min_x, double max_x,
		unsigned int num_steps_x,
		double min_y, double max_y,
		unsigned int num_steps_y)
{
	point_array_t *pa;
	unsigned int i;

	pa = malloc(sizeof(*pa));
	if (!pa)
		return NULL;
	pa->min_x = min_x;
	pa->max_x = max_x;
	pa->num_steps_x = num_steps_x;
	pa->min_y = min_y;
	pa->max_y = max_y;
	pa->num_steps_y = num_steps_y;

	pa->array = malloc(sizeof(*(pa->array)) * num_steps_x);
	for (i = 0; i < num_steps_x; i++) {
		pa->array[i] = malloc(sizeof(**(pa->array)) * num_steps_y);
	}

	return pa;
}

static inline void destruct_point_array(point_array_t *pa)
{
	unsigned int i;

	if (!pa)
		return;

	for (i = 0; i < pa->num_steps_x; i++) {
		free(pa->array[i]);
	}

	free(pa->array);
	free(pa);
}

static inline void copy_part_point_array(point_array_t *pd, point_array_t *ps,
		unsigned int first_idx)
{
	unsigned int i, j, k;

	assert(ps->num_steps_x < pd->num_steps_x);
	assert(ps->num_steps_y == pd->num_steps_y);

	for (i = first_idx, j = 0; j < ps->num_steps_x; i++, j++) {
		for (k = 0; k < ps->num_steps_y; k++)
			pd->array[i][k] = ps->array[j][k];
	}
}

static inline void point_array_module_print_json(point_array_t *pa)
{
	unsigned int i, j;
	double min_z, max_z;

	fprintf(out_file_desc, "experimentData = {\n");

	fprintf(out_file_desc, "minX : %f, maxX : %f, minY : %f, maxY : %f,\n",
			pa->min_x, pa->max_x, pa->min_y, pa->max_y);

	fprintf(out_file_desc, "data : [");

	min_z = max_z = pa->array[0][0].module;
	for (i = 0; i < pa->num_steps_x; i++) {
		fprintf(out_file_desc, "[");
		for (j = 0; j < pa->num_steps_y; j++) {
			min_z = (pa->array[i][j].module < min_z) ?
				pa->array[i][j].module : min_z;
			max_z = (pa->array[i][j].module > max_z) ?
				pa->array[i][j].module : max_z;
			fprintf(out_file_desc, "%f", pa->array[i][j].module);
			if (j != pa->num_steps_y - 1)
				fprintf(out_file_desc, ", ");
		}
		fprintf(out_file_desc, "]");
		if (i != pa->num_steps_x - 1)
			fprintf(out_file_desc, ", ");
		fprintf(out_file_desc, "\n");
	}

	fprintf(out_file_desc, "], \n");
	fprintf(out_file_desc, "minZ: %f, maxZ: %f\n", min_z, max_z);
	fprintf(out_file_desc, "};");
}

static inline void point_array_phase_print_json(point_array_t *pa)
{
	unsigned int i, j;
	double min_z, max_z;

	fprintf(out_file_desc, "experimentData = {\n");

	fprintf(out_file_desc, "minX : %f, maxX : %f, minY : %f, maxY : %f,\n",
			pa->min_x, pa->max_x, pa->min_y, pa->max_y);

	fprintf(out_file_desc, "data : [");

	min_z = max_z = pa->array[0][0].phase;
	for (i = 0; i < pa->num_steps_x; i++) {
		fprintf(out_file_desc, "[");
		for (j = 0; j < pa->num_steps_y; j++) {
			min_z = (pa->array[i][j].phase < min_z) ?
				pa->array[i][j].phase : min_z;
			max_z = (pa->array[i][j].phase > max_z) ?
				pa->array[i][j].phase : max_z;
			fprintf(out_file_desc, "%f", pa->array[i][j].phase);
			if (j != pa->num_steps_y - 1)
				fprintf(out_file_desc, ", ");
		}
		fprintf(out_file_desc, "]");
		if (i != pa->num_steps_x - 1)
			fprintf(out_file_desc, ", ");
		fprintf(out_file_desc, "\n");
	}

	fprintf(out_file_desc, "], \n");
	fprintf(out_file_desc, "minZ: %f, maxZ: %f\n", min_z, max_z);
	fprintf(out_file_desc, "};");
}

#endif /* _WAVECAT_POINT_ARRAY_H_ */
