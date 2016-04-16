#ifndef _WAVECAT_CGI_H_
#define _WAVECAT_CGI_H_

extern unsigned int cgi_mode;
extern FILE *out_file_desc;

#define CGI_ERROR(str)                                      \
{                                                           \
	if (cgi_mode)                                       \
		fprintf(out_file_desc, "Error: %s\n", str); \
} while (0)

#endif
