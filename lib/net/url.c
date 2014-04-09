#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char cth(char code)
{
	static char hex[] = "0123456789abcdef";
	return hex[code & 15];
}

static char htc(char ch)
{
	return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/**
 * url_encode() - encode URL string
 * @src - pointer to source string
 *
 * return converted URL string (allocated in the function)
 *
 * Returned value should be freed after usage.
 */
char *url_encode(char *src)
{
	char *ps, *pd, *dst;
	/* Allocate memory for maximum possible character array. */
	dst = malloc(strlen(src) * 3 + 1);

	ps = src;
	pd = dst;
	while (ps) {
		switch (*ps) {
			case '-':
			case '_':
			case '.':
			case '~':
				*pd++ = *ps;
				break;
			case ' ':
				*pd++ = '+';
				break;
			default:
				if (isalnum(*ps)) {
					*pd++ = *ps;
				} else {
					*pd++ = '%';
					*pd++ = cth(*ps >> 4);
					*pd++ = cth(*ps & 15);
				}
		}
		ps++;
	}

	return dst;
}

/**
 * url_decode() - decode URL string
 * @src - pointer to source string
 *
 * return converted URL string (allocated in the function)
 *
 * Returned value should be freed after usage.
 */
char *url_decode(char *src)
{
	char *ps, *pd, *dst;
	dst = malloc(strlen(src) + 1);

	ps = src;
	pd = dst;
	while (ps) {
		switch (*ps) {
			case '%':
				if (ps[1] && ps[2]) {
					*pd++ = htc(ps[1]) << 4 |
						htc(ps[2]);
					ps += 2;
				}
				break;
			case '+':
				*pd++ = ' ';
				break;
			default:
				*pd++ = *ps;
		}
		ps++;
	}
	*pd = '\0';

	return dst;
}
