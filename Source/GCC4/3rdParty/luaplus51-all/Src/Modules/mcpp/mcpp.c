#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>
#include "mcpp/src/mcpp_lib.h"

static int l_preprocess(lua_State* L) {
	int argc;
	int i;
	char** argv;
	int result;

	luaL_checktype(L, 1, LUA_TTABLE);

	argc = lua_objlen(L, 1);
	for (i = 1; i <= argc; ++i) {
		lua_pushinteger(L, i);
		lua_gettable(L, 1);
		if (lua_type(L, -1) != LUA_TSTRING)
			luaL_argerror(L, 1, "expected a table of separate string arguments");
		lua_pop(L, 1);
	}

	argv = malloc((argc + 1) * sizeof(char*));
	argv[0] = NULL;
	for (i = 1; i <= argc; ++i) {
		lua_pushinteger(L, i);
		lua_gettable(L, 1);
		argv[i] = malloc(lua_objlen(L, -1) + 1);
		strcpy(argv[i], lua_tostring(L, -1));
		lua_pop(L, 1);
	}

    mcpp_use_mem_buffers( 1);           /* enable memory output */
	result = mcpp_lib_main(argc + 1, argv);
	lua_pushinteger(L, result);
	lua_pushstring(L, mcpp_get_mem_buffer( OUT)); /* get the output       */
	lua_pushstring(L, mcpp_get_mem_buffer( ERR)); /* get the diagnostics  */

    mcpp_use_mem_buffers(1);           /* free the memory */
    mcpp_use_mem_buffers(0);           /* enable memory output */

	for (i = 1; i < argc; ++i) {
		free(argv[i]);
	}
	free(argv);

	return 3;
}


static const struct luaL_reg mcpp_funcs[] = {
	{ "preprocess", l_preprocess },
	{ NULL, NULL }
};


LUALIB_API int luaopen_mcpp(lua_State* L) {
	luaL_register(L, "mcpp", mcpp_funcs);
	return 1;
}
