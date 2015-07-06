/*
** $Id: ltable.h,v 2.10.1.1 2007/12/27 13:02:25 roberto Exp $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#ifndef ltable_h
#define ltable_h

#include "lobject.h"

NAMESPACE_LUA_BEGIN

#define gnode(t,i)	(&(t)->node[i])
#if LUAPLUS_EXTENSIONS
#define gkey(n)		((TValue*)(&(n)->i_key.nk))
#else
#define gkey(n)		(&(n)->i_key.nk)
#endif /* LUAPLUS_EXTENSIONS */
#define gval(n)		(&(n)->i_val)
#define gnext(n)	((n)->i_key.nk.next)

#define key2tval(n)	(&(n)->i_key.tvk)


#if LUA_REFCOUNT
Node *luaH_getkey (Table *t, const TValue *key);
void luaH_removekey (lua_State *L, Table *t, Node *n);
#endif /* LUA_REFCOUNT */
LUAI_FUNC const TValue *luaH_getnum (Table *t, int key);
LUAI_FUNC TValue *luaH_setnum (lua_State *L, Table *t, int key);
LUAI_FUNC const TValue *luaH_getstr (Table *t, TString *key);
LUAI_FUNC TValue *luaH_setstr (lua_State *L, Table *t, TString *key);
#if LUA_WIDESTRING
LUAI_FUNC const TValue *luaH_getwstr (Table *t, TString *key);
LUAI_FUNC TValue *luaH_setwstr (lua_State *L, Table *t, TString *key);
#endif /* LUA_WIDESTRING */
LUAI_FUNC const TValue *luaH_get (Table *t, const TValue *key);
LUAI_FUNC TValue *luaH_set (lua_State *L, Table *t, const TValue *key);
LUAI_FUNC Table *luaH_new (lua_State *L, int narray, int lnhash);
LUAI_FUNC void luaH_resizearray (lua_State *L, Table *t, int nasize);
LUAI_FUNC void luaH_free (lua_State *L, Table *t);
LUAI_FUNC int luaH_next (lua_State *L, Table *t, StkId key);
LUAI_FUNC int luaH_getn (Table *t);


#if defined(LUA_DEBUG)
LUAI_FUNC Node *luaH_mainposition (const Table *t, const TValue *key);
LUAI_FUNC int luaH_isdummy (Node *n);
#endif

NAMESPACE_LUA_END

#endif
