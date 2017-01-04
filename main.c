#include <kernel/core/catastrophe.h>
#include <kernel/core/catastrophe_parallel.h>
#include <kernel/core/profiling.h>
#include <kernel/interface/command_line.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "thirdparty/sigie/sigie.h"

int json_input(const char *json_str);
int plugin_loaddir(const char *dir_name);

unsigned int cgi_mode = 0;
FILE *out_file_desc;

/* This port will be used by SCGI mode by default. */
#define SCGI_PORT 8000

int handle_basic(char *input)
{
	PROFILING_DEFINE_TIMESTAMP(ts_before);
	PROFILING_DEFINE_TIMESTAMP(ts_after);

	PROFILING_SAVE_TIMESTAMP(ts_before);
	PROFILING_SAVE_CYCLES(clc_before);

	json_input(input);

	PROFILING_SAVE_CYCLES(clc_after);
	PROFILING_SAVE_TIMESTAMP(ts_after);

	PROFILING_PRINT_CYCLES_DIFFERENCE(clc_before, clc_after);
	PROFILING_PRINT_TIMESTAMP_DIFFERENCE_MS(ts_before, ts_after);

	return 0;
}

static void print_scgi_header(void)
{
	char *response = "Status: 200 OK\r\nContent-Type: text/plain\r\n\r\n";
	fprintf(out_file_desc, "%s", response);
}

int handle_scgi(void)
{
	struct sigie_buffer *buffer = NULL;
	struct sigie_connection *conn;
	int io_sock_fd = -1;
	int err = 0;
	int ret;

	cgi_mode = 1;

	conn = sigie_connection_create(SCGI_PORT);
	if (!conn)
		return 1;

	while (1) {
		buffer = sigie_buffer_create();
		if (!buffer) {
			err = 2;
			goto out;
		}

		io_sock_fd = sigie_accept(conn);
		if (-1 == io_sock_fd) {
			err = 3;
			goto out;
		}

		fprintf(stderr, "Connection accepted.\n");

		ret = sigie_receive(io_sock_fd, buffer);
		if (-1 == ret) {
			err = 4;
			goto out;
		}

		ret = sigie_receive_content(io_sock_fd, buffer);
		if (-1 == ret) {
			err = 5;
			goto out;
		}

		out_file_desc = fdopen(io_sock_fd, "w");
		if (!out_file_desc) {
			perror("Cannot open IO file descriptor.\n");
			err = 6;
			goto out;
		}

		print_scgi_header();

		{
			char *cont, *term, *top;
			cont = term = sigie_buffer_get_content(buffer);
			if (cont[0] != '-') {
				fprintf(stderr, "Incorrect form data.\n");
				fclose(out_file_desc);
				goto out;
			}
			for (; *cont != '\r'; cont++);
			*cont = '\0';
			cont += 2;
			top = strstr(cont, term);
			if (NULL == top) {
				fprintf(stderr,
				"Unable to find a temination string.\n");
				fclose(out_file_desc);
				goto out;
			}
			*top = '\0';
			cont = strstr(cont, "\r\n\r\n");
			if (NULL == cont) {
				fprintf(stderr,
				"Unable to find start of content.\n");
				fclose(out_file_desc);
				goto out;
			}
			cont += 4;
			fprintf(stderr, "Content: %s\n", cont);
			handle_basic(cont);
		}

		fclose(out_file_desc);
		sigie_buffer_destroy(buffer);

		fprintf(stderr, "Connection closed.\n");

		io_sock_fd = -1;
		buffer = NULL;
	}

out:
	if (-1 != io_sock_fd)
		sigie_close(io_sock_fd);

	if (buffer)
		sigie_buffer_destroy(buffer);

	sigie_connection_destroy(conn);

	return err;
}

static void print_cgi_header(void)
{
	printf("Content-Type:text/plain;charset=us-ascii\n\n");
}

int handle_cgi(void)
{
	char *env_string;
	char *input;
	unsigned int len;
	size_t ret;

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

	handle_basic(input);

	free(input);

	return 0;
}

int main(int argc, char **argv)
{

	plugin_loaddir("plugins");
	out_file_desc = stdout;

	switch (argc) {
	case 1:
		return handle_cgi();
	case 2:
		if (0 == strcmp("--scgi", argv[1])) {
			return handle_scgi();
		} else {
			return handle_basic(argv[1]);
		}
	default:
		fprintf(stderr, "Incorrect number of arguments\n");
		return 1;
	}

	return 0;
}
