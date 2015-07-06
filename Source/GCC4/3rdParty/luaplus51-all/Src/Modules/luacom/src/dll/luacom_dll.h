
#include "lua.h"

extern "C" int luacom_openlib(lua_State* L);

#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
extern "C" int luaopen_luacom(lua_State* L);
#endif


