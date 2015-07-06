/*
** $Id: lvm.h,v 2.5.1.1 2007/12/27 13:02:25 roberto Exp $
** Lua virtual machine
** See Copyright Notice in lua.h
*/

#ifndef lvm_h
#define lvm_h


#include "ldo.h"
#include "lobject.h"
#include "ltm.h"

NAMESPACE_LUA_BEGIN

#define tostring(L,o) ((ttype(o) == LUA_TSTRING) || (luaV_tostring(L, o)))
#if LUA_WIDESTRING
#define towstring(L,o) ((ttype(o) == LUA_TWSTRING) || (luaV_towstring(L, o)))
#endif /* LUA_WIDESTRING */

#if LUA_REFCOUNT        
#define tonumber(o,n)	(ttype(o) == LUA_TNUMBER || \
                         (((o) = luaV_tonumber(L,o,n)) != NULL))
#else
#define tonumber(o,n)	(ttype(o) == LUA_TNUMBER || \
                         (((o) = luaV_tonumber(o,n)) != NULL))
#endif /* LUA_REFCOUNT */

#define equalobj(L,o1,o2) \
	(ttype(o1) == ttype(o2) && luaV_equalval(L, o1, o2))

#if LUA_EXT_RESUMABLEVM
#define GETPC(L)	(cast(const Instruction *, L->ctx))
#define SAVEPC(L, pc)	L->ctx = cast(void *, (pc))
#endif /* LUA_EXT_RESUMABLEVM */


LUAI_FUNC int luaV_lessthan (lua_State *L, const TValue *l, const TValue *r);
LUAI_FUNC int luaV_equalval (lua_State *L, const TValue *t1, const TValue *t2);
#if LUA_REFCOUNT        
LUAI_FUNC const TValue *luaV_tonumber (lua_State *L, const TValue *obj, TValue *n);
#else
LUAI_FUNC const TValue *luaV_tonumber (const TValue *obj, TValue *n);
#endif /* LUA_REFCOUNT */
LUAI_FUNC int luaV_tostring (lua_State *L, StkId obj);
#if LUA_WIDESTRING
LUAI_FUNC int luaV_towstring (lua_State *L, StkId obj);
#endif /* LUA_WIDESTRING */
LUAI_FUNC void luaV_gettable (lua_State *L, const TValue *t, TValue *key,
                                            StkId val);
LUAI_FUNC void luaV_settable (lua_State *L, const TValue *t, TValue *key,
                                            StkId val);
#if LUA_EXT_RESUMABLEVM
LUAI_FUNC int luaV_execute (lua_State *L);
LUAI_FUNC void luaV_resume (lua_State *L);
#else
LUAI_FUNC void luaV_execute (lua_State *L, int nexeccalls);
#endif /* LUA_EXT_RESUMABLEVM */
LUAI_FUNC void luaV_concat (lua_State *L, int total, int last);

NAMESPACE_LUA_END

#endif
