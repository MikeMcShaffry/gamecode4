/*
** $Id: lstring.h,v 1.43 2005/04/25 19:24:10 roberto Exp $
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
#define sizewstring(s)	(sizeof(union TString)+((s)->len+1)*sizeof(lua_WChar))

#define sizeudata(u)	(sizeof(union Udata)+(u)->len)

#define luaS_new(L, s)	(luaS_newlstr(L, s, strlen(s)))
#define luaWS_new(L, s)	(luaS_newlwstr(L, s, lua_WChar_len(s)))
#define luaS_newliteral(L, s)	(luaS_newlstr(L, "" s, \
                                 (sizeof(s)/sizeof(char))-1))

#define luaS_fix(s)	l_setbit((s)->tsv.marked, FIXEDBIT)

LUAI_FUNC void luaS_resize (lua_State *L, int newsize);
LUAI_FUNC Udata *luaS_newudata (lua_State *L, size_t s, Table *e);
LUAI_FUNC TString *luaS_newlstr (lua_State *L, const char *str, size_t l);
LUAI_FUNC TString *luaS_newlwstr (lua_State *L, const lua_WChar *str, size_t l);

NAMESPACE_LUA_END

#endif
