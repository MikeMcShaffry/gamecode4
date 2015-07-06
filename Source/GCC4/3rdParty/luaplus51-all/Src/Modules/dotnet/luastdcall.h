// steffenj: replaced all occurances of LUA_API with LUA_DLLEXPORT due to "macro redefinition" and
// "inconsistent dll linkage" errors ...
// there's probably a "correct" way to solve this but right now I prefer the one that works :)

extern "C" {
#include "../../LuaPlus/src/lua.h"
}

/*
** stdcall C function
*/

typedef int (__stdcall *lua_stdcallCFunction) (lua_State *L);

#if 0

/*
** push stdcall C function
*/
void lua_pushstdcallcfunction(lua_State *L, lua_stdcallCFunction fn);

/*
** safe tostring
*/
void lua_safetostring(lua_State *L,int index,char* buffer);

/*
** check metatable
*/

int luaL_checkmetatable(lua_State *L,int index);

int luanet_tonetobject(lua_State *L,int index);

void luanet_newudata(lua_State *L,int val);

void *luanet_gettag(void);

int luanet_rawnetobj(lua_State *L,int index);

int luanet_checkudata(lua_State *L,int index,const char *meta);
#endif