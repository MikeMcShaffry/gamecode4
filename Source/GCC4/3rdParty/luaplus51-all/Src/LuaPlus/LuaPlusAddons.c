///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2010 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#include <string.h>
#include "src/lua.h"
#include "src/lauxlib.h"
#include "src/lobject.h"
#include "src/lstring.h"
#include "src/ltable.h"
#include "src/ldo.h"
#include <assert.h>

NAMESPACE_LUA_BEGIN

// lvm.c

#ifndef lua_number2wstr
#include <stdio.h>
#define lua_number2wstr(s,n)    sprintf((s), LUA_NUMBER_FMT, (n))
#endif

Table *getcurrenv (lua_State *L);

#if LUA_WIDESTRING

int luaV_towstring (lua_State *L, TValue *obj) {
  if (ttype(obj) != LUA_TNUMBER)
    return 0;
  else {
    char s[32];  /* 16 digits, sign, point and \0  (+ some extra...) */
    lua_WChar ws[32];  /* 16 digits, sign, point and \0  (+ some extra...) */
	char* ptr = s;
	lua_WChar* wptr = ws;
	lua_number2wstr(s, nvalue(obj));  /* convert `s' to number */
	while (*ptr != 0)
		*wptr++ = *ptr++;
	*wptr = 0;
    setwsvalue2s(L, obj, luaWS_new(L, ws));
    return 1;
  }
}

#endif /* LUA_WIDESTRING */

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// lapi.c
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifndef api_check
#define api_check(L, o)		/*{ assert(o); }*/
#endif

#undef api_checknelems
#define api_checknelems(L, n)	api_check(L, (n) <= (L->top - L->ci->base))

#undef api_incr_top
#define api_incr_top(L) \
	{if (L->top >= L->ci->top) luaD_checkstack(L, 1); L->top++;}
//#define api_incr_top(L)   (api_check(L, L->top<L->ci->top), L->top++)

TValue *index2adr (lua_State *L, int idx);

extern TValue *luaA_indexAcceptable (lua_State *L, int index);
extern TValue *negindex (lua_State *L, int index);

#if LUA_WIDESTRING

LUA_API int lua_wload (lua_State *L, lua_Reader reader, void *data,
                      const char *chunkname) {
  ZIO z;
  int status;
  lua_lock(L);
  if (!chunkname) chunkname = "?";
  luaZ_init(L, &z, reader, data);
  z.isWide = 1;
  status = luaD_protectedparser(L, &z, chunkname);
  lua_unlock(L);
  return status;
}


typedef struct LoadWS {
  const lua_WChar *s;
  size_t size;
} LoadWS;


static const char *getWS (lua_State *L, void *ud, size_t *size) {
  LoadWS *ls = (LoadWS *)ud;
  (void)L;
  if (ls->size == 0) return NULL;
  *size = ls->size;
  ls->size = 0;
  return (const char*)ls->s;
}


LUALIB_API int luaL_loadwbuffer (lua_State *L, const lua_WChar *buff, size_t size,
                                const char *name) {
  LoadWS ls;
  ls.s = buff;
  ls.size = size * sizeof(lua_WChar);
  return lua_wload(L, getWS, &ls, name);
}


LUALIB_API int (luaL_loadwstring) (lua_State *L, const lua_WChar *s) {
  return luaL_loadwbuffer(L, s, lua_WChar_len(s), "buffer");
}

/*
** {======================================================
** compatibility code
** =======================================================
*/

static void tag_errorhelper (lua_State *L, int narg, int tag) {
  luaL_typerror(L, narg, lua_typename(L, tag));
}


LUALIB_API const lua_WChar *luaL_checklwstring (lua_State *L, int narg, size_t *len) {
  const lua_WChar *s = lua_tolwstring(L, narg, len);
  if (!s) tag_errorhelper(L, narg, LUA_TWSTRING);
  return s;
}


LUALIB_API void luaL_addlwstring (luaL_Buffer *B, const lua_WChar *s, size_t l) {
  while (l--)
    luaL_addwchar(B, *s++);
}


LUALIB_API void luaL_addwstring (luaL_Buffer *B, const lua_WChar *s) {
  luaL_addlwstring(B, s, lua_WChar_len(s));
}


LUALIB_API void luaL_wbuffinit (lua_State *L, luaL_Buffer *B) {
  B->L = L;
  B->p = B->buffer;
  B->isWide = 1;
  B->lvl = 0;
}


LUA_API int lua_iswstring (lua_State *L, int index) {
  int t = lua_type(L, index);
  return (t == LUA_TWSTRING || t == LUA_TNUMBER);
}


LUA_API const lua_WChar *lua_tolwstring (lua_State *L, int idx, size_t *len) {
  StkId o = index2adr(L, idx);
  if (!ttiswstring(o)) {
    lua_lock(L);  /* `luaV_tostring' may create a new string */
    if (!luaV_towstring(L, o)) {  /* conversion failed? */
      if (len != NULL) *len = 0;
      lua_unlock(L);
      return NULL;
    }
    luaC_checkGC(L);
    o = index2adr(L, idx);  /* previous call may reallocate the stack */
    lua_unlock(L);
  }
  if (len != NULL) *len = twsvalue(o)->len;
  return wsvalue(o);
}


LUA_API const lua_WChar *lua_towstring(lua_State *L, int i) {
  return lua_tolwstring(L, (i), NULL);
}


LUA_API void lua_pushlwstring (lua_State *L, const lua_WChar *s, size_t len) {
  lua_lock(L);
  setwsvalue(L, L->top, luaS_newlwstr(L, s, len));
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API void lua_pushwstring (lua_State *L, const lua_WChar *s) {
  if (s == NULL)
    lua_pushnil(L);
  else
    lua_pushlwstring(L, s, lua_WChar_len(s));
}

#endif /* LUA_WIDESTRING */

NAMESPACE_LUA_END
