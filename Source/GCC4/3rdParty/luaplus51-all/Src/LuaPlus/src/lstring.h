/*
** $Id: lstring.h,v 1.43.1.1 2007/12/27 13:02:25 roberto Exp $
** String table (keep all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#ifndef lstring_h
#define lstring_h


#include "lgc.h"
#include "lobject.h"
#include "lstate.h"

NAMESPACE_LUA_BEGIN

#define sizestring(s)	(sizeof(union TString)+((s)->len+1)*sizeof(char))
#if LUA_WIDESTRING
#define sizewstring(s)	(sizeof(union TString)+((s)->len+1)*sizeof(lua_WChar))
#endif /* LUA_WIDESTRING */

#define sizeudata(u)	(sizeof(union Udata)+(u)->len)

#define luaS_new(L, s)	(luaS_newlstr(L, s, strlen(s)))
#if LUA_WIDESTRING
#define luaWS_new(L, s)	(luaS_newlwstr(L, s, lua_WChar_len(s)))
#endif /* LUA_WIDESTRING */
#define luaS_newliteral(L, s)	(luaS_newlstr(L, "" s, \
                                 (sizeof(s)/sizeof(char))-1))

#if LUA_REFCOUNT
#define luaS_fix(s)	(l_setbit((s)->tsv.marked, FIXEDBIT), s->tsv.ref = 1)
#else
#define luaS_fix(s)	l_setbit((s)->tsv.marked, FIXEDBIT)
#endif /* LUA_REFCOUNT */

LUAI_FUNC void luaS_resize (lua_State *L, int newsize);
LUAI_FUNC Udata *luaS_newudata (lua_State *L, size_t s, Table *e);
LUAI_FUNC TString *luaS_newlstr (lua_State *L, const char *str, size_t l);
#if LUA_WIDESTRING
LUAI_FUNC TString *luaS_newlwstr (lua_State *L, const lua_WChar *str, size_t l);
#endif /* LUA_WIDESTRING */

NAMESPACE_LUA_END

#endif
