/******************************************************************************
* Copyright (C) 2007 Juergen Hoetzel
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/
#include <stdio.h>

#include "Lua-utility.h"

const char* luaL_getstrfield(lua_State* L, const char* key) {
  const char *val;
  lua_getfield(L, -1, key);
  val = lua_isnil(L, -1)? NULL : lua_tostring(L, -1);
  lua_pop(L, 1);
  return val;
}

const char* luaL_getlstrfield(lua_State* L, const char* key, int *len) {
  const char *val;
  lua_getfield(L, -1, key);
  val = lua_isnil(L, -1)? NULL : lua_tolstring(L, -1, len);
  lua_pop(L, 1);
  return val;
}

void _stackDump (lua_State *L, const char *file, int line) {
  int i;
  int top = lua_gettop(L);
  printf("%s:%d: Stackdump\n", file, line);
  for (i = 1; i <= top; i++) {  /* repeat for each level */
	int t = lua_type(L, i);
	printf("%d: ", i);
	switch (t) {
	  case LUA_TSTRING:  /* strings */
		printf("`%s'\n", lua_tostring(L, i));
		break;

	  case LUA_TBOOLEAN:  /* booleans */
		printf(lua_toboolean(L, i) ? "true" : "false");
		break;

	  case LUA_TNUMBER:  /* numbers */
		printf("%g\n", lua_tonumber(L, i));
		break;

	  default:  /* other values */
		printf("%s\n", lua_typename(L, t));
		break;

	}
	printf("  ");  /* put a separator */
  }
  printf("\n");  /* end the listing */
}
