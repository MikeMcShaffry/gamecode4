#include "lua.h"

/*
** stdcall C function
*/

typedef int (*lua_stdcallCFunction) (lua_State *L);

/*
** push stdcall C function
*/
LUA_API void lua_pushstdcallcfunction(lua_State *L, lua_stdcallCFunction fn);

/*
** safe tostring
*/
LUA_API void lua_safetostring(lua_State *L,int index,char* buffer);

/*
** check metatable
*/

LUA_API int luaL_checkmetatable(lua_State *L,int index);

LUA_API int luanet_tonetobject(lua_State *L,int index);

LUA_API void luanet_newudata(lua_State *L,int val);

LUA_API void *luanet_gettag(void);

LUA_API int luanet_rawnetobj(lua_State *L,int index);

LUA_API int luanet_checkudata(lua_State *L,int index,const char *meta);
