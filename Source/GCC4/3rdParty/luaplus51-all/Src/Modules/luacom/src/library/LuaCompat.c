/*
 * LuaCompat.c
 *
 *  Implementation of the class LuaCompat,
 *  which tries to hide almost all diferences
 *  between Lua versions
 *
 */


#include <assert.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>

#if !(defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501)
#ifdef COMPAT51
#include "compat-5.1.h"
#endif
#endif

#include "LuaCompat.h"

#define UNUSED(x) (void)(x)

#define LUASTACK_SET(L) int __LuaAux_luastack_top_index = lua_gettop(L)

#ifdef NDEBUG
#define LUASTACK_CLEAN(L, n) lua_settop(L, __LuaAux_luastack_top_index + n)
#else
#define LUASTACK_CLEAN(L, n) assert((__LuaAux_luastack_top_index + n) == lua_gettop(L))
#endif

#define LUACOMPAT_ERRORMESSAGE "__luacompat_errormesage"


/* Lua 5 version of the API */

void luaCompat_openlib(lua_State* L, const char* libname, const struct luaL_reg* funcs)
{ /* lua5 */
  LUASTACK_SET(L);


#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
  luaL_register(L, libname, funcs);
#elif defined(COMPAT51)
  luaL_module(L, libname, funcs, 0);
#else
  luaL_openlib(L, libname, funcs, 0);
#endif

  LUASTACK_CLEAN(L, 1);
}

void luaCompat_error(lua_State* L, const char* message)
{ /* lua5 */
  lua_pushstring(L, message);
  lua_error(L);
}

int luaCompat_call(lua_State* L, int nargs, int nresults, const char** pErrMsg)
{ /* lua5 */
  int result = lua_pcall(L, nargs, nresults, 0);

  if(result)
  {
    if(pErrMsg)
      *pErrMsg = lua_tostring(L, -1);

    lua_pop(L, 1);
  }

  return result;
}


void luaCompat_newLuaType(lua_State* L, const char* module, const char* type)
{ /* lua5 */
  LUASTACK_SET(L);

  lua_newtable(L);

  /* stores the typename in the Lua table, allowing some reflexivity */
  lua_pushstring(L, "type");
  lua_pushstring(L, type);
  lua_settable(L, -3);

  /* stores type in the module */
  luaCompat_moduleSet(L, module, type);

  LUASTACK_CLEAN(L, 0);
}

void luaCompat_pushTypeByName(lua_State* L,
                               const char* module_name,
                               const char* type_name)
{ /* lua5 */
  LUASTACK_SET(L);

  luaCompat_moduleGet(L, module_name, type_name);

  if(lua_isnil(L, -1))
  {
    lua_pop(L, 1);
    luaCompat_newLuaType(L, module_name, type_name);
    luaCompat_moduleGet(L, module_name, type_name);    
  }

  LUASTACK_CLEAN(L, 1);
}


int luaCompat_newTypedObject(lua_State* L, void* object)
{ /* lua5 */
  LUASTACK_SET(L);

  luaL_checktype(L, -1, LUA_TTABLE);

  lua_boxpointer(L, object);

  lua_insert(L, -2);

  lua_setmetatable(L, -2);

  LUASTACK_CLEAN(L, 0);

  return 1;
}

void luaCompat_setType(lua_State* L, int index)
{ /* lua5 */

  LUASTACK_SET(L);

  lua_setmetatable(L, index);    

  LUASTACK_CLEAN(L,-1);
}



void luaCompat_moduleSet(lua_State* L, const char* module, const char* key)
{ /* lua5 */
  LUASTACK_SET(L);

  lua_pushstring(L, module);
  lua_gettable(L, LUA_REGISTRYINDEX);

  lua_pushstring(L, key);
  lua_pushvalue(L, -3);
  lua_settable(L, -3);

  lua_pop(L, 2);

  LUASTACK_CLEAN(L, -1);
}

void luaCompat_moduleGet(lua_State* L, const char* module, const char* key)
{ /* lua5 */
  LUASTACK_SET(L);

  lua_pushstring(L, module);
  lua_gettable(L, LUA_REGISTRYINDEX);

  lua_pushstring(L, key);
  lua_gettable(L, -2);

  lua_remove(L, -2);

  LUASTACK_CLEAN(L, 1);
}


void* luaCompat_getTypedObject(lua_State* L, int index)
{ /* lua5 */
  void **pObj = (void **) lua_touserdata(L, index);

  void *Obj= *pObj;

  return Obj;
}


int luaCompat_isOfType(lua_State* L, const char* module, const char* type)
{ /* lua5 */
  int result = 0;
  LUASTACK_SET(L);

  luaCompat_getType(L, -1);
  luaCompat_pushTypeByName(L, module, type);

  result = (lua_equal(L, -1, -2) ? 1 : 0);

  lua_pop(L, 2);

  LUASTACK_CLEAN(L, 0);

  return result;
}

void luaCompat_getType(lua_State* L, int index)
{ /* lua5 */
  LUASTACK_SET(L);
  int result = lua_getmetatable(L, index);

  if(!result)
    lua_pushnil(L);

  LUASTACK_CLEAN(L, 1);
}

const void* luaCompat_getType2(lua_State* L, int index)
{ /* lua5 */
  const void* result = 0;

  LUASTACK_SET(L);

  if(!lua_getmetatable(L, index))
    lua_pushnil(L);

  result = lua_topointer(L, -1);
  lua_pop(L, 1);

  LUASTACK_CLEAN(L, 0);

  return result;
}


void luaCompat_handleEqEvent(lua_State* L)
{ /* lua5 */
  LUASTACK_SET(L);

  lua_pushstring(L, "__eq");
  lua_insert(L, -2);

  lua_settable(L, -3);

  LUASTACK_CLEAN(L, -1);
}


void luaCompat_handleGettableEvent(lua_State* L)
{ /* lua5 */
  // there is no gettable_event in Lua5 with the semantics of
  // Lua4
}

void luaCompat_handleSettableEvent(lua_State* L)
{ /* lua5 */
  LUASTACK_SET(L);

  lua_pushstring(L, "__newindex");
  lua_insert(L, -2);

  lua_settable(L, -3);

  LUASTACK_CLEAN(L, -1);

}


void luaCompat_handleNoIndexEvent(lua_State* L)
{ /* lua5 */
  LUASTACK_SET(L);

  lua_pushstring(L, "__index");
  lua_insert(L, -2);

  lua_settable(L, -3);

  LUASTACK_CLEAN(L, -1);
}


void luaCompat_handleGCEvent(lua_State* L)
{ /* lua5 */
  LUASTACK_SET(L);

  lua_pushstring(L, "__gc");
  lua_insert(L, -2);

  lua_settable(L, -3);

  LUASTACK_CLEAN(L, -1);
}

void luaCompat_handleFuncCallEvent(lua_State* L)
{ /* lua5 */
  LUASTACK_SET(L);

  lua_pushstring(L, "__call");
  lua_insert(L, -2);

  lua_settable(L, -3);

  LUASTACK_CLEAN(L, -1);
}


int luaCompat_upvalueIndex(lua_State* L, int which, int num_upvalues)
{ /* lua5 */
  UNUSED(num_upvalues);

  return lua_upvalueindex(which);
}

int luaCompat_getNumParams(lua_State* L, int num_upvalues)
{ /* lua5 */
  UNUSED(num_upvalues);
  return lua_gettop(L);
}

void luaCompat_moduleCreate(lua_State* L, const char* module)
{ /* lua5 */
  LUASTACK_SET(L);

  lua_pushstring(L, module);
  lua_gettable(L, LUA_REGISTRYINDEX);

  if(lua_isnil(L, -1))
  {
    lua_pop(L, 1);
    lua_newtable(L);
    lua_pushstring(L, module);
    lua_pushvalue(L, -2);

    lua_settable(L, LUA_REGISTRYINDEX);
  }

  LUASTACK_CLEAN(L, 1);
}

void luaCompat_pushPointer(lua_State* L, void *pointer)
{ /* lua5 */
  lua_pushlightuserdata(L, pointer);
}

void* luaCompat_getPointer(lua_State* L, int index)
{ /* lua5 */
  if(!lua_islightuserdata(L, index))
    return NULL;

  return lua_touserdata(L, index);
}

void luaCompat_pushBool(lua_State* L, int value)
{ /* lua5 */
  LUASTACK_SET(L);

  lua_pushboolean(L, value);

  LUASTACK_CLEAN(L, 1);
}

void luaCompat_pushCBool(lua_State* L, int value)
{ /* lua5 */
  LUASTACK_SET(L);

  lua_pushboolean(L, value);

  LUASTACK_CLEAN(L, 1);
}

int luaCompat_toCBool(lua_State* L, int index)
{ /* lua5 */
  int value = lua_toboolean(L, index);

  return value;
}


void luaCompat_needStack(lua_State* L, long size)
{ /* lua5 */
  lua_checkstack(L, size);
}


void luaCompat_getglobal(lua_State* L)
{ /* lua5 */
  lua_gettable(L, LUA_GLOBALSINDEX);
}

void luaCompat_setglobal(lua_State* L)
{ /* lua5 */
  lua_settable(L, LUA_GLOBALSINDEX);
}


int luaCompat_checkTagToCom(lua_State *L, int luaval) 
{ /* lua5 */
  /* unused: int has; */

  if(!lua_getmetatable(L, luaval)) return 0;

  lua_pushstring(L, "__tocom");
  lua_gettable(L, -2);
  if(lua_isnil(L,-1)) {
    lua_pop(L, 2);
	return 0;
  }

  lua_remove(L,-2);
  return 1;
}

