#ifndef _WAVECAT_CGI_H_
#define _WAVECAT_CGI_H_

extern unsigned int cgi_mode;

#define CGI_ERROR(str)                      \
{                                           \
	if (cgi_mode)                       \
		printf("Error: %s\n", str); \
} while (0)

#endif
