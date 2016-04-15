#include <kernel/core/catastrophe.h>
#include <kernel/core/catastrophe_parallel.h>
#include <kernel/core/profiling.h>
#include <kernel/interface/command_line.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int json_input(const char *json_str);

unsigned int cgi_mode = 0;

static void print_cgi_header(void)
{
	printf("Content-Type:text/plain;charset=us-ascii\n\n");
}

int plugin_loaddir(const char *dir_name);

int main(int argc, char **argv)
{
	char *env_string;
	char *input;
	unsigned int len;
	size_t ret;

	plugin_loaddir("plugins");

	switch (argc) {
	case 1:
	{
		/* The application is used through CGI. */
		print_cgi_header();

		cgi_mode = 1;

		env_string = getenv("CONTENT_LENGTH");
		if (!env_string) {
			fprintf(stderr, "No content\n");
			return 1;
		}

		len = atoi(env_string);
		input = malloc(len + 1);
		if (!input) {
			fprintf(stderr, "Unable to allocate input buffer\n");
			return 1;
		}

		ret = fread(input, 1, len, stdin);
		if (!ret) {
			fprintf(stderr, "Unable to read data from stdin\n");
			return 1;
		}

		input[len] = '\0';

		fprintf(stderr, "Input length: %u\n", len);
		fprintf(stderr, "Input: %s\n", input);

		PROFILING_DEFINE_TIMESTAMP(ts_before);
		PROFILING_DEFINE_TIMESTAMP(ts_after);

		PROFILING_SAVE_TIMESTAMP(ts_before);
		PROFILING_SAVE_CYCLES(clc_before);
		json_input(input);
		PROFILING_SAVE_CYCLES(clc_after);
		PROFILING_SAVE_TIMESTAMP(ts_after);

		PROFILING_PRINT_CYCLES_DIFFERENCE(clc_before, clc_after);
		PROFILING_PRINT_TIMESTAMP_DIFFERENCE_MS(ts_before, ts_after);

		free(input);

		break;
	}
	case 2:
	{
		PROFILING_DEFINE_TIMESTAMP(ts_before);
		PROFILING_DEFINE_TIMESTAMP(ts_after);

		PROFILING_SAVE_TIMESTAMP(ts_before);
		PROFILING_SAVE_CYCLES(clc_before);
		json_input(argv[1]);
		PROFILING_SAVE_CYCLES(clc_after);
		PROFILING_SAVE_TIMESTAMP(ts_after);

		PROFILING_PRINT_CYCLES_DIFFERENCE(clc_before, clc_after);
		PROFILING_PRINT_TIMESTAMP_DIFFERENCE_MS(ts_before, ts_after);

		break;
	}
	default:
		fprintf(stderr, "Incorrect number of arguments\n");
		return 1;
	}

	return 0;
}
