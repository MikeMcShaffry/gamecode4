/*
 * LuaCompat.h
 *
 *  Library that tries to hide almost all diferences
 *  between Lua versions
 *
 */

#ifndef __LUACOMPAT_H
#define __LUACOMPAT_H

void luaCompat_openlib(lua_State* L, const char* libname, const struct luaL_reg* funcs);
void luaCompat_error(lua_State* L, const char* message);
int luaCompat_call(lua_State* L, int nargs, int nresults, const char** pErrMsg);

void luaCompat_newLuaType(lua_State* L, const char* module_name, const char* name);
void luaCompat_pushTypeByName(lua_State* L, const char* module, const char* type_name);
int luaCompat_newTypedObject(lua_State* L, void* object);
void luaCompat_setType(lua_State* L, int index);
void* luaCompat_getTypedObject(lua_State* L, int index);
int luaCompat_isOfType(lua_State* L, const char* module, const char* type);
void luaCompat_getType(lua_State* L, int index);
const void* luaCompat_getType2(lua_State* L, int index);

void luaCompat_pushPointer(lua_State* L, void *pointer);
void* luaCompat_getPointer(lua_State* L, int index);

void luaCompat_pushCBool(lua_State* L, int value);
int luaCompat_toCBool(lua_State* L, int index);
void luaCompat_pushBool(lua_State* L, int value);

void luaCompat_handleEqEvent(lua_State* L);
void luaCompat_handleGettableEvent(lua_State* L);
void luaCompat_handleSettableEvent(lua_State* L);
void luaCompat_handleNoIndexEvent(lua_State* L);
void luaCompat_handleGCEvent(lua_State* L);
void luaCompat_handleFuncCallEvent(lua_State* L);


void luaCompat_moduleCreate(lua_State* L, const char* module);
void luaCompat_moduleSet(lua_State* L, const char* module, const char* key);
void luaCompat_moduleGet(lua_State* L, const char* module, const char* key);

int luaCompat_upvalueIndex(lua_State* L, int which, int num_upvalues);
int luaCompat_getNumParams(lua_State* L, int num_upvalues);

void luaCompat_needStack(lua_State* L, long size);

void luaCompat_setglobal(lua_State* L);
void luaCompat_getglobal(lua_State* L);

int luaCompat_checkTagToCom(lua_State *L, int luaval);

#ifdef __cplusplus
extern "C"
{
#endif
#include <lua.h>
#ifdef __cplusplus
}
#endif

// Lua 5.1 compatibility
#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
#define LUA5
#define luaL_check_lstr luaL_checklstring
#define luaL_check_number luaL_checknumber
#define luaL_arg_check luaL_argcheck
#define luaL_opt_lstr luaL_optlstring
#define lua_boxpointer(L,u) \
    (*(void **)(lua_newuserdata(L, sizeof(void *))) = (u)) 
#endif

#endif /* __LUACOMPAT_H */

