/*
** Compat-5.1 release 2
** Copyright Kepler Project 2004-2005 (http://www.keplerproject.org/compat)
*/

#ifndef COMPAT_H

LUALIB_API void luaL_module(lua_State *L, const char *libname,
                                       const luaL_reg *l, int nup);
#define luaL_openlib luaL_module

#endif
