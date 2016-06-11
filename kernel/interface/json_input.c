#include <stdio.h>
#include <stdlib.h>

#include <kernel/core/config.h>
#include <kernel/core/catastrophe.h>
#include <kernel/core/catastrophe_parallel.h>

#include "jsmn.h"

/* Some JSON node type checking macros. */
#define TOK_PRIMITIVE(tok) ((tok).type == JSMN_PRIMITIVE)
#define TOK_OBJECT(tok)    ((tok).type == JSMN_OBJECT)
#define TOK_ARRAY(tok)     ((tok).type == JSMN_ARRAY)
#define TOK_STRING(tok)    ((tok).type == JSMN_STRING)

/* Maximum variable or parameter name length. */
#define MAX_NAME_LEN       80

/* Derive these definitions from kernel/core/config.h */
#define MAX_PARAMETERS     CONFIG_CAT_MAX_PARAMETERS
#define MAX_VARIABLES      CONFIG_CAT_MAX_VARIABLES

/*
 * substrcpy() - copy a substring to some other place.
 *
 * @dst   : pointer to destination string
 * @src   : pointer to source string
 * @start : offset to start of substring
 * @end   : offset to end of substring
 * @dsize : size of destination string
 *
 * Returns -1 on fail and 0 on success.
 */
static inline int substrcpy(char *dst, const char *src,
		int start, int end, int dsize)
{
	int len;
	len = end - start;

	if (start >= end)
		return -1;
	if (len + 1 > dsize)
		return -1;

	memcpy(dst, src + start, len);
	dst[len] = '\0';

	return 0;
}

/* Enumeration of parser states controlling the parsing sequence. */
enum jsi_parse_state {
	PARSE_TOP_KEY = 0,
	PARSE_NAME_VALUE,
	PARSE_VAR_KEY,
	PARSE_VAR_VALUE,
	PARSE_PAR_KEY,
	PARSE_PAR_VALUE,
	PARSE_PAR_ARR_0,
	PARSE_PAR_ARR_1,
	PARSE_PAR_ARR_2,
	PARSE_MODE
};

static char *state_str[] = {
	"PARSE_TOP_KEY",
	"PARSE_NAME_VALUE",
	"PARSE_VAR_KEY",
	"PARSE_VAR_VALUE",
	"PARSE_PAR_KEY",
	"PARSE_PAR_VALUE",
	"PARSE_PAR_ARR_0",
	"PARSE_PAR_ARR_1",
	"PARSE_PAR_ARR_2",
	"PARSE_MODE"
};

struct jsi_parse_cont {
	parameter_t       parameter[MAX_PARAMETERS];
	unsigned int      param_index;
	variable_t        variable[MAX_VARIABLES];
	unsigned int      var_index;
	char              name[MAX_NAME_LEN];
	int               is_phase;

	enum jsi_parse_state state;
};

static inline int jsi_parse_cont_init(struct jsi_parse_cont *jpc)
{
	if (!jpc)
		return -1;

	jpc->param_index = 0;
	jpc->var_index   = 0;
	jpc->is_phase    = 0;
	jpc->state       = PARSE_TOP_KEY;

	return 0;
}

static char *jsi_token_string[] = {
	"JSMN_PRIMITIVE",
	"JSMN_OBJECT",
	"JSMN_ARRAY",
	"JSMN_STRING"
};

static int
jsi_token_expect( const jsmntok_t *token,
                  jsmntype_t type )
{
	if (token->type != type) {
		fprintf(stderr, "Token error! %s token is expected.\n",
				jsi_token_string[type]);
		CGI_ERROR("Incorrect token");
		return -1;
	}

	return 0;
}

static int
jsi_parse_prim( struct jsi_parse_cont *jpc,
		const jsmntok_t *tokens,
		const int nr_tokens,
		const char *str,
		int *token_index );

static int
jsi_parse_arr( struct jsi_parse_cont *jpc,
	       const jsmntok_t *tokens,
	       const int nr_tokens,
	       const char *str,
	       int *token_index )
{
	char temp[MAX_NAME_LEN];

	int nr_elems;
	int err = 0;

	nr_elems = tokens[*token_index - 1].size;

	if (jpc->state != PARSE_PAR_VALUE) {
		err = -1;
		fprintf(stderr, "Incorrect state (arr)\n");
		CGI_ERROR("Cannot parse the array");
		goto out;
	}

	if (nr_elems != 3) {
		err = -1;
		fprintf(stderr, "Incorrect number of array elements\n");
		CGI_ERROR("Incorrect number of array elements");
		goto out;
	}

	jpc->state = PARSE_PAR_ARR_0;

	while (nr_elems--) {
		if (tokens[*token_index].type != JSMN_PRIMITIVE) {
			err = -1;
			fprintf(stderr, "Incorrect token type (array)\n");
			CGI_ERROR("Cannot parse the array");
			goto out;
		}

		err = jsi_parse_prim(jpc, tokens, nr_tokens, str, token_index);
		if (err)
			goto out;
	}

	jpc->state = PARSE_PAR_KEY;
out:
	return err;
}

static int
jsi_parse_str( struct jsi_parse_cont *jpc,
	       const jsmntok_t *tokens,
	       const int nr_tokens,
	       const char *str,
	       int *token_index )
{
	char temp[MAX_NAME_LEN];
	int err = 0;

	substrcpy(temp, str,
			tokens[*token_index].start,
			tokens[*token_index].end,
			MAX_NAME_LEN);

	if (jpc->state == PARSE_NAME_VALUE)
		strcpy(jpc->name, temp);
	else if (jpc->state == PARSE_MODE) {
		if (0 == strcmp(temp, "phase"))
			jpc->is_phase = 1;
	} else {
		err = -1;
		fprintf(stderr, "Incorrect state (string)\n");
		CGI_ERROR("Cannot parse the string");
	}

	(*token_index)++;
	jpc->state = PARSE_TOP_KEY;

	return err;
}

static int
jsi_parse_prim( struct jsi_parse_cont *jpc,
		const jsmntok_t *tokens,
		const int nr_tokens,
		const char *str,
		int *token_index )
{
	char temp[MAX_NAME_LEN];
	int err = 0;

	substrcpy(temp, str,
			tokens[*token_index].start,
			tokens[*token_index].end,
			MAX_NAME_LEN);

	switch (jpc->state) {
		/* This is a top-level, a key is parsed. */
		case PARSE_TOP_KEY:
			if (0 == strcmp(temp, "name")) {
				jpc->state = PARSE_NAME_VALUE;
			} else if (0 == strcmp(temp, "params")) {
				jpc->state = PARSE_PAR_KEY;
			} else if (0 == strcmp(temp, "vars")) {
				jpc->state = PARSE_VAR_KEY;
			} else if (0 == strcmp(temp, "mode")) {
				jpc->state = PARSE_MODE;
			} else {
				err = -1;
				fprintf(stderr, "Incorrect top key\n");
				CGI_ERROR("Incorrect key at the top level");
				goto out;
			}
			break;
		case PARSE_VAR_VALUE:
			jpc->variable[jpc->var_index].cur_value =
				atof(temp);
			jpc->var_index++;
			jpc->state = PARSE_VAR_KEY;
			break;
		case PARSE_PAR_VALUE:
			jpc->parameter[jpc->param_index].min_value =
			jpc->parameter[jpc->param_index].max_value =
			jpc->parameter[jpc->param_index].cur_value =
				atof(temp);
			jpc->param_index++;
			jpc->state = PARSE_PAR_KEY;
			break;
		/* Parsing the lowest boundary of the parameter. */
		case PARSE_PAR_ARR_0:
			jpc->parameter[jpc->param_index].min_value =
				atof(temp);
			jpc->state = PARSE_PAR_ARR_1;
			break;
		/* Parsing the highest boundary of the parameter. */
		case PARSE_PAR_ARR_1:
			jpc->parameter[jpc->param_index].max_value =
				atof(temp);
			jpc->state = PARSE_PAR_ARR_2;
			break;
		/* Parsing number of steps for the parameter. */
		case PARSE_PAR_ARR_2:
			jpc->parameter[jpc->param_index].num_steps =
				atoi(temp);
			jpc->param_index++;
			jpc->state = PARSE_PAR_KEY;
			break;
		case PARSE_PAR_KEY:
			strcpy(jpc->parameter[jpc->param_index].sym_name,
				temp);
			jpc->state = PARSE_PAR_VALUE;
			break;
		case PARSE_VAR_KEY:
			strcpy(jpc->variable[jpc->var_index].sym_name,
				temp);
			jpc->state = PARSE_VAR_VALUE;
			break;
		default:
			err = -1;
			fprintf(stderr, "Incorrect state (primitive)\n");
			fprintf(stderr, "state: %s\n", state_str[jpc->state]);
			CGI_ERROR("Cannot parse the primitive");
			goto out;
	}

	(*token_index)++;
out:
	return err;
}

static int
jsi_parse_obj( struct jsi_parse_cont *jpc,
	       const jsmntok_t *tokens,
               const int nr_tokens,
	       const char *str,
	       int *token_index )
{
	int nr_elems;
	int ret = 0;

	nr_elems = tokens[*token_index - 1].size;
	nr_elems *= 2; /* Due to degradation in jsmn library. */

	if (jpc->state != PARSE_TOP_KEY &&
		jpc->state != PARSE_PAR_KEY &&
		jpc->state != PARSE_VAR_KEY)
	{
		fprintf(stderr, "Incorrect state (obj)\n");
		CGI_ERROR("Cannot parse the object");
		ret = -1;
		goto out;
	}

	while (nr_elems--) {
		switch (tokens[*token_index].type) {
			case JSMN_PRIMITIVE:
				ret = jsi_parse_prim(jpc, tokens, nr_tokens,
						str, token_index);
				break;
			case JSMN_OBJECT:
				(*token_index)++;
				ret = jsi_parse_obj(jpc, tokens, nr_tokens,
						str, token_index);
				break;
			case JSMN_ARRAY:
				(*token_index)++;
				ret = jsi_parse_arr(jpc, tokens, nr_tokens,
						str, token_index);
				break;
			case JSMN_STRING:
				ret = jsi_parse_str(jpc, tokens, nr_tokens,
						str, token_index);
				break;
			default:
				fprintf(stderr,
					"Token error. Incorrect token type.\n");
				CGI_ERROR("Incorrect type of the token");
				return -1;
		}

		if (ret)
			goto out;
	}

	jpc->state = PARSE_TOP_KEY;

out:
	return ret;
}

static int
jsi_parse( struct jsi_parse_cont *jpc,
           const jsmntok_t *tokens,
           const int nr_tokens,
	   const char *str )
{
	int token_index = 0;
	int res;

	jsi_parse_cont_init(jpc);

	/* We have a JSON object as an input */
	res = jsi_token_expect(&tokens[0], JSMN_OBJECT);
	if (res)
		return -1;

	token_index++;
	res = jsi_parse_obj(jpc, tokens, nr_tokens, str, &token_index);
	if (res)
		return -1;

	return 0;
}

int json_input(const char *json_str)
{
	jsmn_parser parser;

	struct jsi_parse_cont jpc;

	catastrophe_desc_t       *catastrophe_desc = NULL;
	catastrophe_t *catastrophe;

#define NRTOKENS  50
	jsmntok_t tokens[NRTOKENS];
	int ret;

	jsmn_init(&parser);

	ret = jsmn_parse(&parser, json_str, strlen(json_str),
			tokens, NRTOKENS);
	switch (ret) {
		case JSMN_ERROR_INVAL:
		case JSMN_ERROR_NOMEM:
		case JSMN_ERROR_PART:
			fprintf(stderr,
				"Error: Unable to parse json data\n");
			CGI_ERROR("Unable to parse json data");
			return -1;
		default:
			if (!ret) {
				fprintf(stderr,
					"Error: no tokens\n");
				CGI_ERROR("No tokens found\n");
				return -1;
			}
			assert(ret > 0);
			/* The variable "ret" now contains number of actually
			 * used tokens
			 */
			ret = jsi_parse(&jpc, tokens, NRTOKENS, json_str);
			if (ret)
				return -1;
	}

	/* Check parsed values symantically */
	if (!strlen(jpc.name)) {
		fprintf(stderr, "Symantic error: empty name\n");
		CGI_ERROR("Name cannot be empty");
		return -1;
	}

	catastrophe_desc = find_catastrophe_desc(jpc.name);

	if (catastrophe_desc) {
		fprintf(stderr, "Catastrophe descriptor is found\n");
		if (catastrophe_desc->num_parameters != jpc.param_index) {
			fprintf(stderr, "Incorrect number of parameters\n");
			CGI_ERROR("Incorrect number of parameters");
			return -1;
		}
		catastrophe = catastrophe_desc->fabric(catastrophe_desc,
				jpc.parameter, jpc.variable);
		if (!catastrophe)
			return -1;
		if (catastrophe_parallel_loop(catastrophe))
			return -1;
		if (!jpc.is_phase)
			point_array_module_print_json(
				catastrophe->point_array);
		else
			point_array_phase_print_json(
				catastrophe->point_array);
		destruct_catastrophe(catastrophe);
	}  else {
		fprintf(stderr, "Corresponding module is not found\n");
		CGI_ERROR("Module is not found");
		return -1;
	}

	return 0;
}
