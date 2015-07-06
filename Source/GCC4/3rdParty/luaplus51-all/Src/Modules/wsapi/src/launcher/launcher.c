
/*
** Simple Lua interpreter.
** This program is used to run a Lua file with the same name but the
** extension (that should be .lua).
** It creates a Lua state, opens all its standard libraries, and run
** the Lua file in a protected environment just to redirect the error
** messages to stdout and stderr.
**
** $Id: launcher.c,v 1.4 2007/12/20 18:15:48 mascarenhas Exp $
*/

#include <string.h>
#include <stdlib.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include <windows.h>
#include <io.h>
#include <fcntl.h>

/*
** Report error message.
** Assumes that the error message is on top of the stack.
*/
static int report (lua_State *L) {
	fprintf (stderr, "lua: fatal error: `%s'\n", lua_tostring (L, -1));
	fflush (stderr);
	printf ("Content-type: text/plain\n\nConfiguration fatal error: see error log!\n");
	printf ("%s", lua_tostring(L, -1));
	return 1;
}

static int runlua (lua_State *L, char *name) {
	int err_func;
	
	lua_pushliteral(L, "debug");
	lua_rawget(L, LUA_GLOBALSINDEX);  /* get traceback function */
        lua_pushliteral(L, "traceback");
        lua_gettable(L, -2);
	err_func = lua_gettop (L);
	return (luaL_loadfile (L, name)) || (lua_pcall (L, 0, 0, err_func));
}

static DWORD GetModulePath( HINSTANCE hInst, LPTSTR pszBuffer, DWORD dwSize )
//
//	Return the size of the path in bytes.
{
	DWORD dwLength = GetModuleFileName( hInst, pszBuffer, dwSize );
	if( dwLength )
	{
		while( dwLength && pszBuffer[ dwLength ] != '.' )
		{
			dwLength--;
		}

		if( dwLength )
			pszBuffer[ dwLength ] = '\000';
	}
	return dwLength;
}

/*
** MAIN
*/
int main (int argc, char *argv[]) {
	char name[ MAX_PATH ];
	DWORD dwLength;
	lua_State *L = lua_open();
	(void)argc; /* avoid "unused parameter" warning */
	dwLength = GetModulePath( NULL, name, MAX_PATH );
	if(dwLength) {
		luaL_openlibs(L);
		_setmode(_fileno(stdin), _O_BINARY);
		_setmode(_fileno(stdout), _O_BINARY);
		strcat(name, ".cgi");
		if (runlua (L, name)) {
			report (L);
			lua_close (L);
			return EXIT_FAILURE;
		}
	}
	lua_close (L);
	return EXIT_SUCCESS;
}
