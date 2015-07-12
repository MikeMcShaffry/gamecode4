///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2005 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://wwhiz.com/LuaPlus/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#include <string.h>
#define LUA_CORE
#include "src/lua.h"
#include "src/lauxlib.h"
#include "src/lobject.h"
#include "src/lstring.h"
#include "src/ltable.h"
#include "src/ldo.h"
#include <assert.h>

NAMESPACE_LUA_BEGIN

#define luaplus_assert(e) /* empty */

// lvm.c

#ifndef lua_number2wstr
#include <stdio.h>
#define lua_number2wstr(s,n)    sprintf((s), LUA_NUMBER_FMT, (n))
#endif

Table *getcurrenv (lua_State *L);

static size_t lua_WChar_len(const lua_WChar* str)
{
	const lua_WChar* ptr = str;
	while (*ptr != 0)
	{
		ptr++;
	}

	return (size_t)(ptr - str);
}


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

#define api_checknelems(L, n)	api_check(L, (n) <= (L->top - L->ci->base))

#define api_incr_top(L) \
	{if (L->top >= L->ci->top) luaD_checkstack(L, 1); L->top++;}
//#define api_incr_top(L)   (api_check(L, L->top<L->ci->top), L->top++)

TValue *index2adr (lua_State *L, int idx);

extern TValue *luaA_indexAcceptable (lua_State *L, int index);
extern TValue *negindex (lua_State *L, int index);


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
  ls.size = size * 2;
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


static void callalert (lua_State *L, int status) {
  if (status != 0) {
    lua_getglobal(L, "_ALERT");
    if (lua_isfunction(L, -1)) {
      lua_insert(L, -2);
      lua_call(L, 1, 0);
    }
    else {  /* no _ALERT function; print it on stderr */
      fprintf(stderr, "%s\n", lua_tostring(L, -2));
      lua_pop(L, 2);  /* remove error message and _ALERT */
    }
  }
}


static void tag_error (lua_State *L, int narg, int tag) {
  luaL_typerror(L, narg, lua_typename(L, tag));
}


static int aux_do (lua_State *L, int status) {
  if (status == 0) {  /* parse OK? */
    status = lua_pcall(L, 0, LUA_MULTRET, 0);  /* call main */
  }
  callalert(L, status);
  return status;
}


LUALIB_API int lua_dofile (lua_State *L, const char *filename) {
  return aux_do(L, luaL_loadfile(L, filename));
}


LUALIB_API int lua_dobuffer (lua_State *L, const char *buff, size_t size,
                          const char *name) {
  return aux_do(L, luaL_loadbuffer(L, buff, size, name));
}


LUALIB_API int lua_dostring (lua_State *L, const char *str) {
  return lua_dobuffer(L, str, strlen(str), str);
}


LUALIB_API int lua_dowbuffer (lua_State *L, const lua_WChar *buff, size_t size,
                          const char *name) {
  return aux_do(L, luaL_loadwbuffer(L, buff, size, name));
}


LUALIB_API int lua_dowstring (lua_State *L, const lua_WChar *str, const char *name) {
  return lua_dowbuffer(L, str, lua_WChar_len(str), name);
}

/* }====================================================== */

LUALIB_API const lua_WChar *luaL_checklwstring (lua_State *L, int narg, size_t *len) {
  const lua_WChar *s = lua_towstring(L, narg);
  if (!s) tag_error(L, narg, LUA_TWSTRING);
  if (len) *len = lua_strlen(L, narg);
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


LUA_API void lua_getdefaultmetatable(lua_State *L, int type)
{
  lua_lock(L);
//jj  sethvalue(L, L->top, hvalue(defaultmetatypes(L, type)));
  assert(0);
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API void lua_setdefaultmetatable(lua_State *L, int type)
{
  StkId t;
  lua_lock(L);
  api_checknelems(L, 1);
  t = L->top - 1;
  if (ttype(t) == LUA_TTABLE) {  /* `t' is a table? */
//jj	  sethvalue(L, defaultmetatypes(L, type), hvalue(t));
	  assert(0);
  }
  L->top -= 1;
  lua_unlock(L);
}


//#define lua_number2int(i,n)	((i)=(int)(n))

LUA_API int lua_getn (lua_State *L, int index) {
  StkId t;
  const TValue *value;
  int n;
  lua_lock(L);
  t = index2adr(L, index);
  api_check(L, ttype(t) == LUA_TTABLE);
  value = luaH_getstr(hvalue(t), luaS_newliteral(L, "n"));  /* = t.n */
  if (ttype(value) == LUA_TNUMBER) {
    n = (int)value;
  } else {
    Node *nd;
    Table *a = hvalue(t);
    lua_Number max = 0;
    int i;
    i = a->sizearray;
    while (i--) {
      if (ttype(&a->array[i]) != LUA_TNIL)
        break;
    }
    max = i+1;
    i = sizenode(a);
    nd = a->node;
    while (i--) {
      if (ttype(gkey(nd)) == LUA_TNUMBER &&
          ttype(gval(nd)) != LUA_TNIL &&
          nvalue(gkey(nd)) > max)
        max = nvalue(gkey(nd));
      nd++;
    }
    lua_number2int(n, max);
  }
  lua_unlock(L);
  return n;
}


LUA_API void lua_newuserdatabox (lua_State *L, void *ptr) {
  Udata *u;
  lua_lock(L);
  u = luaS_newudata(L, 4, getcurrenv(L));
  u->uv.len = 1;  // user data box bit is set.
  *(void**)(u + 1) = ptr;
  setuvalue(L, L->top, u);
  api_incr_top(L);
  lua_unlock(L);
}

#ifdef _MSC_VER
#pragma warning(disable : 4055)
#pragma warning(disable : 4152)

typedef int (__stdcall *lua_stdcallCFunction) (lua_State *L);

static int stdcall_closure(lua_State *L) {
  lua_stdcallCFunction fn = (lua_stdcallCFunction)lua_touserdata(L, lua_upvalueindex(1));
  return fn(L);
}


LUA_API void lua_pushstdcallcfunction(lua_State *L,lua_stdcallCFunction fn) {
  lua_pushlightuserdata(L, fn);
  lua_pushcclosure(L, stdcall_closure, 1);
}

LUA_API void lua_safetostring(lua_State *L,int index,char* buffer) {
  const char* tmp;
  tmp=lua_tostring(L,index);
  memcpy(buffer,tmp,lua_strlen(L,index)+1);
}

#endif // _MSC_VER

NAMESPACE_LUA_END
