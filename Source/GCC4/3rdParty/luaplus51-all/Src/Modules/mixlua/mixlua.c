/* Copyright (c) 2007-2008 Thomas Lavergne
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>

#define mix_tempsize 4096

static const char *err_badeof = "unexpected end of file";

enum mix_token_e {
	tok_sh,
	tok_d2l, tok_l2d, tok_d2e, tok_e2d,
	tok_data, tok_lua, tok_exp,
	tok_eof
};

typedef struct mix_state_s {
	size_t      size;
	const char *buffer;

	const char *d2l;
	size_t      d2lsize;
	const char *l2d;
	size_t      l2dsize;
	const char *exp;
	size_t      expsize;
	char       *out;
	size_t      outsize;

	size_t      pos;
	int         token;
	char        temp[mix_tempsize];

	const char *error;
} mix_state_t;

/* Read an open bracket of the form [=*[ and return the number of = found.
 * if there is no string like this at the current pos return -1.
 * after this call the cursor is set at the first char after the bracket.
 */
static int mix_bracket_open(mix_state_t *S) {
	size_t pos, cnt;
	if (S->buffer[S->pos] != '[')
		return -1;
	pos = S->pos + 1;
	cnt = 0;
	while (pos < S->size && S->buffer[pos] == '=')
		++pos, ++cnt;
	if (pos == S->size || S->buffer[pos] != '[')
		return -1;
	S->pos = pos + 1;
	return cnt;
}

/* Read a close bracket of the form ]=*] and return the number of = found.
 * if there is no string like this at the current pos return -1.
 * after this call the cursor is set at the first char after the bracket.
 */
static int mix_bracket_close(mix_state_t *S) {
	size_t pos, cnt;
	if (S->buffer[S->pos] != ']')
		return -1;
	pos = S->pos + 1;
	cnt = 0;
	while (pos < S->size && S->buffer[pos] == '=')
		++pos, ++cnt;
	if (pos == S->size || S->buffer[pos] != ']')
		return -1;
	S->pos = pos + 1;
	return cnt;
}

/* Skip a lua string enclosed by type (who can be either \" or \') taking
 * account of escape chars. after a call to this the cursor is set at the
 * first character after the string.
 * Beware that on enter the cursor must be on the first char of the string
 * not on the openning char.
 */
static void mix_skip_string(lua_State *L, mix_state_t *S, char type) {
	while (S->pos < S->size) {
		if (S->buffer[S->pos] == type) {
			++S->pos;
			return;
		}
		if (S->buffer[S->pos] == '\\')
			++S->pos;
		++S->pos;
	}
	luaL_error(L, err_badeof);
}

/* Skip a lua long string wich is enclosed by square bracket and level = signs
 * after the call the cursor is set on the first character after the string.
 * Beware that on enter the cursor must be on the first char of the string
 * not on the openning bracket.
 */
static void mix_skip_lstring(lua_State *L, mix_state_t *S, int level) {
	while (S->pos < S->size) {
		int tmp = mix_bracket_close(S);
		if (tmp == -1)
			++S->pos;
		else if (tmp == level)
			return;
	}
	luaL_error(L, err_badeof);
}

/* Skip a lua comment either one line or long. after the call the cursor is set
 * to the first character after the coment.
 * Beware that on enter the cursor must be set on the first char after the --
 * comment prefix.
 */
static void mix_skip_comment(lua_State *L, mix_state_t *S) {
	int level = mix_bracket_open(S);
	if (level != -1)
		return mix_skip_string(L, S, level);
	while (S->pos < S->size && S->buffer[S->pos] != '\n')
		++S->pos;
}

#define mix_isd2l(s) (!strncmp((s)->d2l, (s)->buffer + (s)->pos, (s)->d2lsize))
#define mix_isl2d(s) (!strncmp((s)->l2d, (s)->buffer + (s)->pos, (s)->l2dsize))
#define mix_isexp(s) (!strncmp((s)->exp, (s)->buffer + (s)->pos, (s)->expsize))

static const char *mix_reader(lua_State *L, void *ud, size_t *size) {
	mix_state_t *S = (mix_state_t *)ud;

	if (S->error != NULL)
		return NULL;

	/* Data chunk are sent to lua by block cause we must escape some char
	 * and we cannot modify the buffer itself. So we must use another
	 * buffer sending escaped block one by one.
	 * If there is no data to send in the chunk, change the state and go
	 * directly to the tok_d2l part.
	 */
	if (S->token == tok_data) {
		*size = 0;
		while (S->pos < S->size) {
			char c = S->buffer[S->pos];
			if (c == S->d2l[0])
				if (mix_isd2l(S))
					break;
			if (c == '\\' || c == '\n' || c == '\'')
				S->temp[(*size)++] = '\\';
			S->temp[(*size)++] = c;
			++S->pos;
			if (*size >= mix_tempsize - 1) {
				if (S->pos == S->size)
					S->token = tok_d2l;
				return S->temp;
			}
		}
		if (S->pos < S->size)
			S->pos += S->d2lsize;
		S->token = tok_d2l;
		if (*size != 0)
			return S->temp;
	}

	/* Send the termination of the string and the function call before
	 * going in tok_lua/tok_exp state or if at eof go to tok_eof.
	 */
	if (S->token == tok_d2l) {
		if (S->pos == S->size) {
			S->token = tok_eof;
		} else if (mix_isexp(S)) {
			S->token = tok_d2e;
			S->pos += S->expsize;
		} else {
			S->token = tok_lua;
		}
		*size = 2;
		return "')";
	}

	/* Send the output function, but without the string start character, so
	 * the result of the lua code will be send as parameters to the output
	 * function.
	 */
	if (S->token == tok_d2e) {
		S->token = tok_exp;
		*size = S->outsize - 1;
		return S->out;
	}

	/* Send a lua chunk in one block to the lua engine. No escaping are
	 * needed here so we can send the full block. Then switch to the
	 * tok_l2d. (this correctly skip all form of lua string and comments)
	 * If no lua code is found, go directly to the tok_l2d part.
	 */
	if (S->token == tok_lua || S->token == tok_exp) {
		size_t old = S->pos;
		while (S->pos < S->size) {
			char c = S->buffer[S->pos];
			if (c == '\'') {
				++S->pos;
				mix_skip_string(L, S, '\'');
			} else if (c == '"') {
				++S->pos;
				mix_skip_string(L, S, '"');
			} else if (c == '[') {
				int level = mix_bracket_open(S);
				if (level != -1)
					mix_skip_lstring(L, S, level);
				else
					++S->pos;
			} else if (c == '-') {
				++S->pos;
				if (S->buffer[S->pos] == '-') {
					++S->pos;
					mix_skip_comment(L, S);
				}
			} else if (c == S->l2d[0]) {
				if (mix_isl2d(S))
					break;
				++S->pos;
			} else {
				++S->pos;
			}
		}
		*size = S->pos - old;
		if (S->pos == S->size) {
			S->error = err_badeof;
			return NULL;
		}
		S->token = S->token == tok_lua ? tok_l2d : tok_e2d;
		S->pos += S->l2dsize;
		if (*size != 0)
			return S->buffer + old;
	}

	/* Send the data output function name and (' that start a data block to
	 * the lua engine and switch to the tok_data state.
	 */
	if (S->token == tok_l2d) {
		S->token = tok_data;
		*size = S->outsize;
		return S->out;
	}

	/* Close the output function call on end of expression and go to the lua
	 * to data transition.
	 */
	if (S->token == tok_e2d) {
		S->token = tok_l2d;
		*size = 1;
		return ")";
	}

	/* If we skipped a sh-bang line we must send an empty line to keep the
	 * lua line counter correct.
	 */
	if (S->token == tok_sh) {
		S->token = tok_l2d;
		*size = 1;
		return "\n";
	}

	return NULL;
}

/* Generic loader function. Load the data found in the state in the lua engine
 */
static int mix_loadbuffer(lua_State *L, mix_state_t *S, const char *name) {
	int res;
	S->pos = 0;

	if (S->size > 0 && S->buffer[0] == '#') {
		while (S->pos < S->size && S->buffer[S->pos] != '\n')
			++S->pos;
		++S->pos;
		S->token = tok_sh;
	} else {
		S->token = tok_l2d;
	}

	S->error = NULL;
	res = lua_load(L, mix_reader, S, name);
	if (S->error != NULL) {
		lua_pushnil(L);
		lua_pushstring(L, S->error);
		return 2;
	} else if (res != 0) {
		lua_pushnil(L);
		lua_insert(L, -2);
		return 2;
	}
	return 1;
}

/* Read the options common to mix_loadbuffer and mix_loadfile and store them in
 * the state.
 */
static void mix_stateopt(lua_State *L, mix_state_t *S) {
	const char *tmp;
	size_t size;

	S->d2l = luaL_checklstring(L, 2, &S->d2lsize);
	if (S->d2lsize == 0)
		luaL_argerror(L, 2, "d2l separator cannot be empty");

	S->l2d = luaL_checklstring(L, 3, &S->l2dsize);
	if (S->l2dsize == 0)
		luaL_argerror(L, 3, "l2d separator cannot be empty");

	S->exp = luaL_optlstring(L, 4, "=", &S->expsize);
	if (S->expsize == 0)
		luaL_argerror(L, 4, "exp separator cannot be empty");

	tmp = luaL_optlstring(L, 5, "io.write", &size);
	S->out = lua_newuserdata(L, size + 2);
	memcpy(S->out, tmp, size);
	S->out[size] = '(';
	S->out[size + 1] = '\'';
	S->outsize = size + 2;
}

static int mix_loadfile(lua_State *L) {
	const char  *filename;
	FILE        *file;
	char        *buffer;
	mix_state_t  S;

	filename = luaL_checkstring(L, 1);
	mix_stateopt(L, &S);

	file = fopen(filename, "r");
	if (file == NULL) {
		lua_pushnil(L);
		lua_pushfstring(L, "cannot open file <%s>", filename);
		return 2;
	}

	fseek(file, 0, SEEK_END);
	S.size = ftell(file);
	fseek(file, 0, SEEK_SET);

	buffer = lua_newuserdata(L, S.size);
	if (fread(buffer, S.size, 1, file) != 1) {
		lua_pushnil(L);
		lua_pushfstring(L, "cannot read file <%>", filename);
		return 2;
	}
	S.buffer = buffer;

	return mix_loadbuffer(L, &S, filename);
}

static int mix_loadstring(lua_State *L) {
	mix_state_t S;

	S.buffer = luaL_checklstring(L, 1, &S.size);
	mix_stateopt(L, &S);

	return mix_loadbuffer(L, &S, NULL);
}

static const luaL_reg mix_lib[] = {
	{"loadfile",   mix_loadfile  },
	{"loadstring", mix_loadstring},
	{NULL,         NULL          }
};

int luaopen_mixlua(lua_State *L) {
	luaL_register(L, "mix", mix_lib);
	return 1;
}

