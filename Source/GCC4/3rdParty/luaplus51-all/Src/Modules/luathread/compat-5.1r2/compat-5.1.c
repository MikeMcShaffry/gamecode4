#include <stdio.h>
#include <string.h>
#include "lua.h"
#include "lauxlib.h"
#include "compat-5.1.h"

static void getfield(lua_State *L, const char *name) {
    const char *end = strchr(name, '.');
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    while (end) {
        lua_pushlstring(L, name, end - name);
        lua_gettable(L, -2);
        lua_remove(L, -2);
        if (lua_isnil(L, -1)) return;
        name = end+1;
        end = strchr(name, '.');
    }
    lua_pushstring(L, name);
    lua_gettable(L, -2);
    lua_remove(L, -2);
}

static void setfield(lua_State *L, const char *name) {
    const char *end = strchr(name, '.');
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    while (end) {
        lua_pushlstring(L, name, end - name);
        lua_gettable(L, -2);
        /* create table if not found */
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            lua_newtable(L);
            lua_pushlstring(L, name, end - name);
            lua_pushvalue(L, -2);
            lua_settable(L, -4);
        }
        lua_remove(L, -2);
        name = end+1;
        end = strchr(name, '.');
    }
    lua_pushstring(L, name);
    lua_pushvalue(L, -3);
    lua_settable(L, -3);
    lua_pop(L, 2);
}

LUALIB_API void luaL_module(lua_State *L, const char *libname,
                              const luaL_reg *l, int nup) {
  if (libname) {
    getfield(L, libname);  /* check whether lib already exists */
    if (lua_isnil(L, -1)) { 
      lua_pop(L, 1); /* get rid of nil */
      lua_newtable(L); /* create namespace for lib */
      getfield(L, "package.loaded"); /* get package.loaded table or create it */
      if (lua_isnil(L, -1)) {
          lua_pop(L, 1);
          lua_newtable(L);
          lua_pushvalue(L, -1);
          setfield(L, "package.loaded");
      }
      else if (!lua_istable(L, -1))
        luaL_error(L, "name conflict for library `%s'", libname);
      lua_pushstring(L, libname);
      lua_pushvalue(L, -3); 
      lua_settable(L, -3); /* store namespace in package.loaded table */
      lua_pop(L, 1); /* get rid of package.loaded table */
      lua_pushvalue(L, -1);
      setfield(L, libname);  /* store namespace it in globals table */
    }
    lua_insert(L, -(nup+1));  /* move library table to below upvalues */
  }
  for (; l->name; l++) {
    int i;
    lua_pushstring(L, l->name);
    for (i=0; i<nup; i++)  /* copy upvalues to the top */
      lua_pushvalue(L, -(nup+1));
    lua_pushcclosure(L, l->func, nup);
    lua_settable(L, -(nup+3));
  }
  lua_pop(L, nup);  /* remove upvalues */
}
