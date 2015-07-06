/*
** $Id: lapi.c,v 2.55.1.5 2008/07/04 18:41:18 roberto Exp $
** Lua API
** See Copyright Notice in lua.h
*/


#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>

#define lapi_c
#define LUA_CORE

#include "lua.h"

#include "lapi.h"
#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "ltm.h"
#include "lundump.h"
#include "lvm.h"

NAMESPACE_LUA_BEGIN

const char lua_ident[] =
  "$Lua: " LUA_RELEASE " " LUA_COPYRIGHT " $\n"
  "$Authors: " LUA_AUTHORS " $\n"
  "$URL: www.lua.org $\n";



#if defined(LUAPLUS_ALL)
#undef api_checknelems
#endif /* LUAPLUS_ALL */
#define api_checknelems(L, n)	api_check(L, (n) <= (L->top - L->base))

#define api_checkvalidindex(L, i)	api_check(L, (i) != luaO_nilobject)

#if defined(LUAPLUS_ALL)
#undef api_incr_top
#endif /* LUAPLUS_ALL */
#define api_incr_top(L)   {api_check(L, L->top < L->ci->top); L->top++;}


#if LUA_FASTREF_SUPPORT

static const TValue *luaH_getinthelper (Table *t, int key) {
  /* (1 <= key && key <= t->sizearray) */
  if (cast(unsigned int, key-1) < cast(unsigned int, t->sizearray))
    return &t->array[key-1];
  else
    return luaH_getnum(t, key);
}


static TValue *luaH_setinthelper (lua_State *L, Table *t, int key) {
  /* (1 <= key && key <= t->sizearray) */
  if (cast(unsigned int, key-1) < cast(unsigned int, t->sizearray))
    return &t->array[key-1];
  else
    return luaH_setnum(L, t, key);
}

#endif /* LUA_FASTREF_SUPPORT */


#if LUAPLUS_EXTENSIONS
TValue *index2adr (lua_State *L, int idx) {
#else
static TValue *index2adr (lua_State *L, int idx) {
#endif /* LUAPLUS_EXTENSIONS */
  if (idx > 0) {
    TValue *o = L->base + (idx - 1);
    api_check(L, idx <= L->ci->top - L->base);
    if (o >= L->top) return cast(TValue *, luaO_nilobject);
    else return o;
  }
  else if (idx > LUA_REGISTRYINDEX) {
    api_check(L, idx != 0 && -idx <= L->top - L->base);
    return L->top + idx;
  }
#if LUA_FASTREF_SUPPORT
  else if (idx <= LUA_FASTREFNIL) {
    if (idx == LUA_FASTREFNIL)
      return &G(L)->fastrefNilValue;
    idx = -idx + LUA_FASTREFNIL - 1;
    return (TValue*)luaH_getinthelper(hvalue(&G(L)->l_refs), idx);
  }
#endif /* LUA_FASTREF_SUPPORT */
  else switch (idx) {  /* pseudo-indices */
    case LUA_REGISTRYINDEX: return registry(L);
    case LUA_ENVIRONINDEX: {
      Closure *func = curr_func(L);
      sethvalue(L, &L->env, func->c.env);
      return &L->env;
    }
    case LUA_GLOBALSINDEX: return gt(L);
    default: {
      Closure *func = curr_func(L);
      idx = LUA_GLOBALSINDEX - idx;
      return (idx <= func->c.nupvalues)
                ? &func->c.upvalue[idx-1]
                : cast(TValue *, luaO_nilobject);
    }
  }
}


#if LUAPLUS_EXTENSIONS
Table *getcurrenv (lua_State *L) {
#else
static Table *getcurrenv (lua_State *L) {
#endif /* LUAPLUS_EXTENSIONS */
  if (L->ci == L->base_ci)  /* no enclosing function? */
    return hvalue(gt(L));  /* use global table as environment */
  else {
    Closure *func = curr_func(L);
    return func->c.env;
  }
}


void luaA_pushobject (lua_State *L, const TValue *o) {
  setobj2s(L, L->top, o);
  api_incr_top(L);
}


LUA_API int lua_checkstack (lua_State *L, int size) {
  int res = 1;
  lua_lock(L);
  if (size > LUAI_MAXCSTACK || (L->top - L->base + size) > LUAI_MAXCSTACK)
    res = 0;  /* stack overflow */
  else if (size > 0) {
    luaD_checkstack(L, size);
    if (L->ci->top < L->top + size)
      L->ci->top = L->top + size;
  }
  lua_unlock(L);
  return res;
}


LUA_API void lua_xmove (lua_State *from, lua_State *to, int n) {
#if LUA_REFCOUNT
  lua_State* L = from;
#endif /* LUA_REFCOUNT */
  int i;
  if (from == to) return;
  lua_lock(to);
  api_checknelems(from, n);
  api_check(from, G(from) == G(to));
  api_check(from, to->ci->top - to->top >= n);
  from->top -= n;
  for (i = 0; i < n; i++) {
    setobj2s(to, to->top++, from->top + i);
#if LUA_REFCOUNT
    luarc_cleanvalue(from->top + i);
#endif /* LUA_REFCOUNT */
  }
  lua_unlock(to);
}


LUA_API void lua_setlevel (lua_State *from, lua_State *to) {
  to->nCcalls = from->nCcalls;
}


LUA_API lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf) {
  lua_CFunction old;
  lua_lock(L);
  old = G(L)->panic;
  G(L)->panic = panicf;
  lua_unlock(L);
  return old;
}


LUA_API lua_State *lua_newthread (lua_State *L) {
  lua_State *L1;
  lua_lock(L);
  luaC_checkGC(L);
  L1 = luaE_newthread(L);
  setthvalue(L, L->top, L1);
  api_incr_top(L);
  lua_unlock(L);
  luai_userstatethread(L, L1);
  return L1;
}



/*
** basic stack manipulation
*/


LUA_API int lua_gettop (lua_State *L) {
  return cast_int(L->top - L->base);
}


LUA_API void lua_settop (lua_State *L, int idx) {
  lua_lock(L);
  if (idx >= 0) {
    api_check(L, idx <= L->stack_last - L->base);
    while (L->top < L->base + idx)
      setnilvalue(L->top++);
#if LUA_REFCOUNT
    luarc_cleanarray(L->base + idx, L->top);
#endif /* LUA_REFCOUNT */
    L->top = L->base + idx;
  }
  else {
    api_check(L, -(idx+1) <= (L->top - L->base));

#if LUA_REFCOUNT
    {
      StkId begin = L->top + idx + 1;
      StkId end = L->top; (void)end;
      if (begin < L->base)
        begin = L->base;
      luarc_cleanarray(begin, end);
    }
    if (-(idx + 1) > (L->top - L->base))
      idx = L->base - L->top - 1;
#endif /* LUA_REFCOUNT */

    L->top += idx+1;  /* `subtract' index (index is negative) */
  }
  lua_unlock(L);
}


LUA_API void lua_remove (lua_State *L, int idx) {
  StkId p;
  lua_lock(L);
  p = index2adr(L, idx);
  api_checkvalidindex(L, p);
  while (++p < L->top) setobjs2s(L, p-1, p);
  L->top--;
#if LUA_REFCOUNT
  luarc_cleanvalue(L->top);
#endif /* LUA_REFCOUNT */
  lua_unlock(L);
}


LUA_API void lua_insert (lua_State *L, int idx) {
  StkId p;
  StkId q;
  lua_lock(L);
  p = index2adr(L, idx);
  api_checkvalidindex(L, p);
  for (q = L->top; q>p; q--) setobjs2s(L, q, q-1);
  setobjs2s(L, p, L->top);
#if LUA_REFCOUNT
  luarc_cleanvalue(L->top);
#endif /* LUA_REFCOUNT */
  lua_unlock(L);
}


LUA_API void lua_replace (lua_State *L, int idx) {
  StkId o;
  lua_lock(L);
  /* explicit test for incompatible code */
  if (idx == LUA_ENVIRONINDEX && L->ci == L->base_ci)
    luaG_runerror(L, "no calling environment");
  api_checknelems(L, 1);
  o = index2adr(L, idx);
  api_checkvalidindex(L, o);
  if (idx == LUA_ENVIRONINDEX) {
    Closure *func = curr_func(L);
    api_check(L, ttistable(L->top - 1));
    func->c.env = hvalue(L->top - 1);
    luaC_barrier(L, func, L->top - 1);
  }
  else {
    setobj(L, o, L->top - 1);
    if (idx < LUA_GLOBALSINDEX)  /* function upvalue? */
      luaC_barrier(L, curr_func(L), L->top - 1);
  }
  L->top--;
#if LUA_REFCOUNT
  luarc_cleanvalue(L->top);
#endif /* LUA_REFCOUNT */
  lua_unlock(L);
}


LUA_API void lua_pushvalue (lua_State *L, int idx) {
  lua_lock(L);
  setobj2s(L, L->top, index2adr(L, idx));
  api_incr_top(L);
  lua_unlock(L);
}



/*
** access functions (stack -> C)
*/


LUA_API int lua_type (lua_State *L, int idx) {
  StkId o = index2adr(L, idx);
  return (o == luaO_nilobject) ? LUA_TNONE : ttype(o);
}


LUA_API const char *lua_typename (lua_State *L, int t) {
  UNUSED(L);
  return (t == LUA_TNONE) ? "no value" : luaT_typenames[t];
}


LUA_API int lua_iscfunction (lua_State *L, int idx) {
  StkId o = index2adr(L, idx);
  return iscfunction(o);
}


LUA_API int lua_isnumber (lua_State *L, int idx) {
  TValue n;
  const TValue *o = index2adr(L, idx);
#if LUA_REFCOUNT
  luarc_newvalue(&n);
#endif /* LUA_REFCOUNT */
  return tonumber(o, &n);
}


LUA_API int lua_isstring (lua_State *L, int idx) {
  int t = lua_type(L, idx);
  return (t == LUA_TSTRING || t == LUA_TNUMBER);
}


LUA_API int lua_isuserdata (lua_State *L, int idx) {
  const TValue *o = index2adr(L, idx);
  return (ttisuserdata(o) || ttislightuserdata(o));
}


LUA_API int lua_rawequal (lua_State *L, int index1, int index2) {
  StkId o1 = index2adr(L, index1);
  StkId o2 = index2adr(L, index2);
  return (o1 == luaO_nilobject || o2 == luaO_nilobject) ? 0
         : luaO_rawequalObj(o1, o2);
}


LUA_API int lua_equal (lua_State *L, int index1, int index2) {
  StkId o1, o2;
  int i;
  lua_lock(L);  /* may call tag method */
  o1 = index2adr(L, index1);
  o2 = index2adr(L, index2);
#if LUA_EXT_RESUMABLEVM
  notresumable(L,
    i = (o1 == luaO_nilobject || o2 == luaO_nilobject) ? 0 : equalobj(L, o1, o2);
  )
#else
  i = (o1 == luaO_nilobject || o2 == luaO_nilobject) ? 0 : equalobj(L, o1, o2);
#endif /* LUA_EXT_RESUMABLEVM */
  lua_unlock(L);
  return i;
}


LUA_API int lua_lessthan (lua_State *L, int index1, int index2) {
  StkId o1, o2;
  int i;
  lua_lock(L);  /* may call tag method */
  o1 = index2adr(L, index1);
  o2 = index2adr(L, index2);
#if LUA_EXT_RESUMABLEVM
  notresumable(L,
    i = (o1 == luaO_nilobject || o2 == luaO_nilobject) ? 0
         : luaV_lessthan(L, o1, o2);
  )
#else
  i = (o1 == luaO_nilobject || o2 == luaO_nilobject) ? 0
       : luaV_lessthan(L, o1, o2);
#endif /* LUA_EXT_RESUMABLEVM */
  lua_unlock(L);
  return i;
}



LUA_API lua_Number lua_tonumber (lua_State *L, int idx) {
  TValue n;
  const TValue *o = index2adr(L, idx);
#if LUA_REFCOUNT
  luarc_newvalue(&n);
#endif /* LUA_REFCOUNT */
  if (tonumber(o, &n))
    return nvalue(o);
  else
    return 0;
}


LUA_API lua_Integer lua_tointeger (lua_State *L, int idx) {
  TValue n;
  const TValue *o = index2adr(L, idx);
#if LUA_REFCOUNT
  luarc_newvalue(&n);
#endif /* LUA_REFCOUNT */
  if (tonumber(o, &n)) {
    lua_Integer res;
    lua_Number num = nvalue(o);
    lua_number2integer(res, num);
    return res;
  }
  else
    return 0;
}


LUA_API int lua_toboolean (lua_State *L, int idx) {
  const TValue *o = index2adr(L, idx);
  return !l_isfalse(o);
}


LUA_API const char *lua_tolstring (lua_State *L, int idx, size_t *len) {
  StkId o = index2adr(L, idx);
  if (!ttisstring(o)) {
    lua_lock(L);  /* `luaV_tostring' may create a new string */
    if (!luaV_tostring(L, o)) {  /* conversion failed? */
      if (len != NULL) *len = 0;
      lua_unlock(L);
      return NULL;
    }
    luaC_checkGC(L);
    o = index2adr(L, idx);  /* previous call may reallocate the stack */
    lua_unlock(L);
  }
  if (len != NULL) *len = tsvalue(o)->len;
  return svalue(o);
}


LUA_API size_t lua_objlen (lua_State *L, int idx) {
  StkId o = index2adr(L, idx);
  switch (ttype(o)) {
    case LUA_TSTRING: return tsvalue(o)->len;
#if LUA_WIDESTRING
    case LUA_TWSTRING: return twsvalue(o)->len;
#endif /* LUA_WIDESTRING */
    case LUA_TUSERDATA: return uvalue(o)->len;
    case LUA_TTABLE: return luaH_getn(hvalue(o));
    case LUA_TNUMBER: {
      size_t l;
      lua_lock(L);  /* `luaV_tostring' may create a new string */
      l = (luaV_tostring(L, o) ? tsvalue(o)->len : 0);
      lua_unlock(L);
      return l;
    }
    default: return 0;
  }
}


LUA_API lua_CFunction lua_tocfunction (lua_State *L, int idx) {
  StkId o = index2adr(L, idx);
  return (!iscfunction(o)) ? NULL : clvalue(o)->c.f;
}


LUA_API void *lua_touserdata (lua_State *L, int idx) {
  StkId o = index2adr(L, idx);
  switch (ttype(o)) {
    case LUA_TUSERDATA: return (rawuvalue(o) + 1);
    case LUA_TLIGHTUSERDATA: return pvalue(o);
    default: return NULL;
  }
}


LUA_API lua_State *lua_tothread (lua_State *L, int idx) {
  StkId o = index2adr(L, idx);
  return (!ttisthread(o)) ? NULL : thvalue(o);
}


LUA_API const void *lua_topointer (lua_State *L, int idx) {
  StkId o = index2adr(L, idx);
  switch (ttype(o)) {
    case LUA_TTABLE: return hvalue(o);
    case LUA_TFUNCTION: return clvalue(o);
    case LUA_TTHREAD: return thvalue(o);
    case LUA_TUSERDATA:
    case LUA_TLIGHTUSERDATA:
      return lua_touserdata(L, idx);
    default: return NULL;
  }
}



/*
** push functions (C -> stack)
*/


LUA_API void lua_pushnil (lua_State *L) {
  lua_lock(L);
  setnilvalue(L->top);
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API void lua_pushnumber (lua_State *L, lua_Number n) {
  lua_lock(L);
  setnvalue(L->top, n);
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API void lua_pushinteger (lua_State *L, lua_Integer n) {
  lua_lock(L);
  setnvalue(L->top, cast_num(n));
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API void lua_pushlstring (lua_State *L, const char *s, size_t len) {
  lua_lock(L);
  luaC_checkGC(L);
  setsvalue2s(L, L->top, luaS_newlstr(L, s, len));
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API void lua_pushstring (lua_State *L, const char *s) {
  if (s == NULL)
    lua_pushnil(L);
  else
    lua_pushlstring(L, s, strlen(s));
}


LUA_API const char *lua_pushvfstring (lua_State *L, const char *fmt,
                                      va_list argp) {
  const char *ret;
  lua_lock(L);
  luaC_checkGC(L);
  ret = luaO_pushvfstring(L, fmt, argp);
  lua_unlock(L);
  return ret;
}


LUA_API const char *lua_pushfstring (lua_State *L, const char *fmt, ...) {
  const char *ret;
  va_list argp;
  lua_lock(L);
  luaC_checkGC(L);
  va_start(argp, fmt);
  ret = luaO_pushvfstring(L, fmt, argp);
  va_end(argp);
  lua_unlock(L);
  return ret;
}


LUA_API void lua_pushcclosure (lua_State *L, lua_CFunction fn, int n) {
  Closure *cl;
  lua_lock(L);
  luaC_checkGC(L);
  api_checknelems(L, n);
  cl = luaF_newCclosure(L, n, getcurrenv(L));
  cl->c.f = fn;
  L->top -= n;
#if LUA_REFCOUNT
  while (n--) {
    setobj2n(L, &cl->c.upvalue[n], L->top+n);
    luarc_cleanvalue(L->top+n);
  }
#else
  while (n--)
    setobj2n(L, &cl->c.upvalue[n], L->top+n);
#endif /* LUA_REFCOUNT */
  setclvalue(L, L->top, cl);
  lua_assert(iswhite(obj2gco(cl)));
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API void lua_pushboolean (lua_State *L, int b) {
  lua_lock(L);
  setbvalue(L->top, (b != 0));  /* ensure that true is 1 */
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API void lua_pushlightuserdata (lua_State *L, void *p) {
  lua_lock(L);
  setpvalue(L->top, p);
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API int lua_pushthread (lua_State *L) {
  lua_lock(L);
  setthvalue(L, L->top, L);
  api_incr_top(L);
  lua_unlock(L);
  return (G(L)->mainthread == L);
}



/*
** get functions (Lua -> stack)
*/


LUA_API void lua_gettable (lua_State *L, int idx) {
  StkId t;
  lua_lock(L);
  t = index2adr(L, idx);
  api_checkvalidindex(L, t);
#if LUA_EXT_RESUMABLEVM
  notresumable(L,
    luaV_gettable(L, t, L->top - 1, L->top - 1);
  )
#else
  luaV_gettable(L, t, L->top - 1, L->top - 1);
#endif /* LUA_EXT_RESUMABLEVM */
  lua_unlock(L);
}


LUA_API void lua_getfield (lua_State *L, int idx, const char *k) {
  StkId t;
  TValue key;
  lua_lock(L);
  t = index2adr(L, idx);
  api_checkvalidindex(L, t);
#if LUA_REFCOUNT
  setsvalue2n(L, &key, luaS_new(L, k));
  luaV_gettable(L, t, &key, L->top);
  setnilvalue(&key);
#else
  setsvalue(L, &key, luaS_new(L, k));
#if LUA_EXT_RESUMABLEVM
  notresumable(L,
    luaV_gettable(L, t, &key, L->top);
  )
#else
  luaV_gettable(L, t, &key, L->top);
#endif /* LUA_EXT_RESUMABLEVM */
#endif /* LUA_REFCOUNT */
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API void lua_rawget (lua_State *L, int idx) {
  StkId t;
  lua_lock(L);
  t = index2adr(L, idx);
  api_check(L, ttistable(t));
  setobj2s(L, L->top - 1, luaH_get(hvalue(t), L->top - 1));
  lua_unlock(L);
}


LUA_API void lua_rawgeti (lua_State *L, int idx, int n) {
  StkId o;
  lua_lock(L);
  o = index2adr(L, idx);
  api_check(L, ttistable(o));
  setobj2s(L, L->top, luaH_getnum(hvalue(o), n));
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API void lua_createtable (lua_State *L, int narray, int nrec) {
  lua_lock(L);
  luaC_checkGC(L);
  sethvalue(L, L->top, luaH_new(L, narray, nrec));
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API int lua_getmetatable (lua_State *L, int objindex) {
  const TValue *obj;
  Table *mt = NULL;
  int res;
  lua_lock(L);
  obj = index2adr(L, objindex);
  switch (ttype(obj)) {
    case LUA_TTABLE:
      mt = hvalue(obj)->metatable;
      break;
    case LUA_TUSERDATA:
      mt = uvalue(obj)->metatable;
      break;
    default:
      mt = G(L)->mt[ttype(obj)];
      break;
  }
  if (mt == NULL)
    res = 0;
  else {
    sethvalue(L, L->top, mt);
    api_incr_top(L);
    res = 1;
  }
  lua_unlock(L);
  return res;
}


LUA_API void lua_getfenv (lua_State *L, int idx) {
  StkId o;
  lua_lock(L);
  o = index2adr(L, idx);
  api_checkvalidindex(L, o);
  switch (ttype(o)) {
    case LUA_TFUNCTION:
      sethvalue(L, L->top, clvalue(o)->c.env);
      break;
    case LUA_TUSERDATA:
      sethvalue(L, L->top, uvalue(o)->env);
      break;
    case LUA_TTHREAD:
      setobj2s(L, L->top,  gt(thvalue(o)));
      break;
    default:
      setnilvalue(L->top);
      break;
  }
  api_incr_top(L);
  lua_unlock(L);
}


/*
** set functions (stack -> Lua)
*/


LUA_API void lua_settable (lua_State *L, int idx) {
  StkId t;
  lua_lock(L);
  api_checknelems(L, 2);
  t = index2adr(L, idx);
  api_checkvalidindex(L, t);
#if LUA_EXT_RESUMABLEVM
  notresumable(L,
    luaV_settable(L, t, L->top - 2, L->top - 1);
  )
#else
  luaV_settable(L, t, L->top - 2, L->top - 1);
#endif /* LUA_EXT_RESUMABLEVM */
  L->top -= 2;  /* pop index and value */
#if LUA_REFCOUNT
  luarc_cleanvalue(L->top);
  luarc_cleanvalue(L->top + 1);
#endif /* LUA_REFCOUNT */
  lua_unlock(L);
}


LUA_API void lua_setfield (lua_State *L, int idx, const char *k) {
  StkId t;
  TValue key;
  lua_lock(L);
  api_checknelems(L, 1);
  t = index2adr(L, idx);
  api_checkvalidindex(L, t);
#if LUA_REFCOUNT
  setsvalue2n(L, &key, luaS_new(L, k));
#else
  setsvalue(L, &key, luaS_new(L, k));
#endif /* LUA_REFCOUNT */
#if LUA_EXT_RESUMABLEVM
  notresumable(L,
    luaV_settable(L, t, &key, L->top - 1);
  )
#else
  luaV_settable(L, t, &key, L->top - 1);
#endif /* LUA_EXT_RESUMABLEVM */
  L->top--;  /* pop value */
#if LUA_REFCOUNT
  setnilvalue(&key);
  luarc_cleanvalue(L->top);
#endif /* LUA_REFCOUNT */
  lua_unlock(L);
}


LUA_API void lua_rawset (lua_State *L, int idx) {
  StkId t;
  lua_lock(L);
  api_checknelems(L, 2);
  t = index2adr(L, idx);
  api_check(L, ttistable(t));
  setobj2t(L, luaH_set(L, hvalue(t), L->top-2), L->top-1);
  luaC_barriert(L, hvalue(t), L->top-1);
  L->top -= 2;
#if LUA_REFCOUNT
  luarc_cleanvalue(L->top);
  luarc_cleanvalue(L->top + 1);
#endif /* LUA_REFCOUNT */
  lua_unlock(L);
}


LUA_API void lua_rawseti (lua_State *L, int idx, int n) {
  StkId o;
  lua_lock(L);
  api_checknelems(L, 1);
  o = index2adr(L, idx);
  api_check(L, ttistable(o));
  setobj2t(L, luaH_setnum(L, hvalue(o), n), L->top-1);
  luaC_barriert(L, hvalue(o), L->top-1);
  L->top--;
#if LUA_REFCOUNT
  luarc_cleanvalue(L->top);
#endif /* LUA_REFCOUNT */
  lua_unlock(L);
}


LUA_API int lua_setmetatable (lua_State *L, int objindex) {
  TValue *obj;
  Table *mt;
  lua_lock(L);
  api_checknelems(L, 1);
  obj = index2adr(L, objindex);
  api_checkvalidindex(L, obj);
  if (ttisnil(L->top - 1))
    mt = NULL;
  else {
    api_check(L, ttistable(L->top - 1));
    mt = hvalue(L->top - 1);
  }
#if LUA_REFCOUNT
  if (mt)
    luarc_addreftable(mt);
#endif /* LUA_REFCOUNT */
  switch (ttype(obj)) {
    case LUA_TTABLE: {
#if LUA_REFCOUNT
      if (hvalue(obj)->metatable)
        luarc_releasetable(L, hvalue(obj)->metatable);
#endif /* LUA_REFCOUNT */
      hvalue(obj)->metatable = mt;
      if (mt)
        luaC_objbarriert(L, hvalue(obj), mt);
      break;
    }
    case LUA_TUSERDATA: {
#if LUA_REFCOUNT
      if (uvalue(obj)->metatable)
        luarc_releasetable(L, uvalue(obj)->metatable);
#endif /* LUA_REFCOUNT */
      uvalue(obj)->metatable = mt;
      if (mt)
        luaC_objbarrier(L, rawuvalue(obj), mt);
      break;
    }
    default: {
#if LUA_REFCOUNT
      if (G(L)->mt[ttype(obj)])
        luarc_releasetable(L, G(L)->mt[ttype(obj)]);
#endif /* LUA_REFCOUNT */
      G(L)->mt[ttype(obj)] = mt;
      break;
    }
  }
  L->top--;
#if LUA_REFCOUNT
  luarc_cleanvalue(L->top);
#endif /* LUA_REFCOUNT */
  lua_unlock(L);
  return 1;
}


LUA_API int lua_setfenv (lua_State *L, int idx) {
  StkId o;
  int res = 1;
  lua_lock(L);
  api_checknelems(L, 1);
  o = index2adr(L, idx);
  api_checkvalidindex(L, o);
  api_check(L, ttistable(L->top - 1));
  switch (ttype(o)) {
#if LUA_REFCOUNT
    case LUA_TFUNCTION: {
      Table *bak = clvalue(o)->c.env;
      clvalue(o)->c.env = hvalue(L->top - 1);
      luarc_addreftable(clvalue(o)->c.env);
      luarc_releasetable(L, bak);
      break;
    }
    case LUA_TUSERDATA: {
      Table *bak = uvalue(o)->env;
      uvalue(o)->env = hvalue(L->top - 1);
      luarc_addreftable(uvalue(o)->env);
      luarc_releasetable(L, bak);
      break;
    }
    case LUA_TTHREAD: {
      TValue bak = thvalue(o)->env;
      sethvalue(L, gt(thvalue(o)), hvalue(L->top - 1));
      luarc_addref(&thvalue(o)->env);
      luarc_release(L, &bak);
      break;
    }
#else
    case LUA_TFUNCTION:
      clvalue(o)->c.env = hvalue(L->top - 1);
      break;
    case LUA_TUSERDATA:
      uvalue(o)->env = hvalue(L->top - 1);
      break;
    case LUA_TTHREAD:
      sethvalue(L, gt(thvalue(o)), hvalue(L->top - 1));
      break;
#endif /* LUA_REFCOUNT */
    default:
      res = 0;
      break;
  }
  if (res) luaC_objbarrier(L, gcvalue(o), hvalue(L->top - 1));
  L->top--;
#if LUA_REFCOUNT
  luarc_cleanvalue(L->top);
#endif /* LUA_REFCOUNT */
  lua_unlock(L);
  return res;
}


/*
** `load' and `call' functions (run Lua code)
*/


#if !LUA_EXT_RESUMABLEVM
#define adjustresults(L,nres) \
    { if (nres == LUA_MULTRET && L->top >= L->ci->top) L->ci->top = L->top; }
#endif /* !LUA_EXT_RESUMABLEVM */

#define checkresults(L,na,nr) \
     api_check(L, (nr) == LUA_MULTRET || (L->ci->top - L->top >= (nr) - (na)))


#if LUA_EXT_RESUMABLEVM

LUA_API void *lua_vcontext (lua_State *L) {
  return L->ctx;
}


LUA_API void lua_vcall (lua_State *L, int nargs, int nresults, void *ctx) {
  int flags;
  lua_lock(L);
  api_checknelems(L, nargs+1);
  checkresults(L, nargs, nresults);
  if (ctx == NULL)
    flags = LUA_NOYIELD | LUA_NOVPCALL;
  else {
    lua_assert(iscfunction(L->ci->func));
    L->ctx = ctx;
    flags = 0;
  }
  luaD_call(L, L->top - (nargs+1), nresults, flags);
  if (L->top > L->ci->top) L->ci->top = L->top;
  lua_unlock(L);
}

#else

LUA_API void lua_call (lua_State *L, int nargs, int nresults) {
  StkId func;
  lua_lock(L);
  api_checknelems(L, nargs+1);
  checkresults(L, nargs, nresults);
  func = L->top - (nargs+1);
  luaD_call(L, func, nresults);
  adjustresults(L, nresults);
  lua_unlock(L);
}

#endif /* LUA_EXT_RESUMABLEVM */


/*
** Execute a protected call.
*/
struct CallS {  /* data to `f_call' */
  StkId func;
  int nresults;
};


#if LUA_EXT_RESUMABLEVM

static int f_call (lua_State *L, void *ud) {
  struct CallS *c = cast(struct CallS *, ud);
  luaD_call(L, c->func, c->nresults, LUA_NOYIELD);
  return 0;
}


LUA_API int lua_vpcall (lua_State *L, int nargs, int nresults,
                        int errfunc, void *ctx) {
  int status;
  lua_lock(L);
  api_checknelems(L, nargs+1);
  checkresults(L, nargs, nresults);
  if (errfunc < 0) errfunc = (L->top - L->base) + errfunc + 1;
  api_check(L, L->base + errfunc <= L->top - (nargs+1));
  if (ctx == NULL || novpcall(L)) {  /* use classic pcall */
    struct CallS c;
    c.func = L->top - (nargs+1);  /* function to be called */
    c.nresults = nresults;
    status = luaD_pcall(L, f_call, &c, savestack(L, c.func),
                        errfunc, ~LUA_NOVPCALL);
  }
  else {  /* else use vpcall */
    luaD_catch(L, errfunc);
    L->ctx = ctx;
    luaD_call(L, L->top - (nargs+1), nresults, 0);
    L->ci->errfunc = 0;
    status = 0;
  }
  if (L->top > L->ci->top) L->ci->top = L->top;
  lua_unlock(L);
  return status;
}

#else

static void f_call (lua_State *L, void *ud) {
  struct CallS *c = cast(struct CallS *, ud);
  luaD_call(L, c->func, c->nresults);
}



LUA_API int lua_pcall (lua_State *L, int nargs, int nresults, int errfunc) {
  struct CallS c;
  int status;
  ptrdiff_t func;
  lua_lock(L);
  api_checknelems(L, nargs+1);
  checkresults(L, nargs, nresults);
  if (errfunc == 0)
    func = 0;
  else {
    StkId o = index2adr(L, errfunc);
    api_checkvalidindex(L, o);
    func = savestack(L, o);
  }
  c.func = L->top - (nargs+1);  /* function to be called */
  c.nresults = nresults;
  status = luaD_pcall(L, f_call, &c, savestack(L, c.func), func);
  adjustresults(L, nresults);
  lua_unlock(L);
  return status;
}

#endif /* LUA_EXT_RESUMABLEVM */

/*
** Execute a protected C call.
*/
struct CCallS {  /* data to `f_Ccall' */
  lua_CFunction func;
  void *ud;
};


#if LUA_EXT_RESUMABLEVM
static int f_Ccall (lua_State *L, void *ud) {
#else
static void f_Ccall (lua_State *L, void *ud) {
#endif /* LUA_EXT_RESUMABLEVM */
  struct CCallS *c = cast(struct CCallS *, ud);
  Closure *cl;
  cl = luaF_newCclosure(L, 0, getcurrenv(L));
  cl->c.f = c->func;
  setclvalue(L, L->top, cl);  /* push function */
  api_incr_top(L);
  setpvalue(L->top, c->ud);  /* push only argument */
  api_incr_top(L);
#if LUA_EXT_RESUMABLEVM
  luaD_call(L, L->top - 2, 0, LUA_NOYIELD);
  return 0;
#else
  luaD_call(L, L->top - 2, 0);
#endif /* LUA_EXT_RESUMABLEVM */
}


LUA_API int lua_cpcall (lua_State *L, lua_CFunction func, void *ud) {
  struct CCallS c;
  int status;
  lua_lock(L);
  c.func = func;
  c.ud = ud;
#if LUA_EXT_RESUMABLEVM
  status = luaD_pcall(L, f_Ccall, &c, savestack(L, L->top), 0, ~LUA_NOVPCALL);
#else
  status = luaD_pcall(L, f_Ccall, &c, savestack(L, L->top), 0);
#endif /* LUA_EXT_RESUMABLEVM */
  lua_unlock(L);
  return status;
}


LUA_API int lua_load (lua_State *L, lua_Reader reader, void *data,
                      const char *chunkname) {
  ZIO z;
  int status;
  lua_lock(L);
  if (!chunkname) chunkname = "?";
  luaZ_init(L, &z, reader, data);
  status = luaD_protectedparser(L, &z, chunkname);
  lua_unlock(L);
  return status;
}


LUA_API int lua_dump (lua_State *L, lua_Writer writer, void *data) {
  int status;
  TValue *o;
  lua_lock(L);
  api_checknelems(L, 1);
  o = L->top - 1;
  if (isLfunction(o))
#if LUA_ENDIAN_SUPPORT
    status = luaU_dump(L, clvalue(o)->l.p, writer, data, 0, '=');
#else
    status = luaU_dump(L, clvalue(o)->l.p, writer, data, 0);
#endif
  else
    status = 1;
  lua_unlock(L);
  return status;
}


#if LUA_ENDIAN_SUPPORT
LUA_API int lua_dumpendian (lua_State *L, lua_Writer writer, void *data, int strip, char endian) {
  int status;
  TValue *o;
  lua_lock(L);
  api_checknelems(L, 1);
  o = L->top - 1;
  if (isLfunction(o))
    status = luaU_dump(L, clvalue(o)->l.p, writer, data, strip, endian);
  else
    status = 1;
  lua_unlock(L);
  return status;
}
#endif


LUA_API int  lua_status (lua_State *L) {
  return L->status;
}


/*
** Garbage-collection function
*/

LUA_API int lua_gc (lua_State *L, int what, int data) {
  int res = 0;
  global_State *g;
  lua_lock(L);
  g = G(L);
  switch (what) {
    case LUA_GCSTOP: {
      g->GCthreshold = MAX_LUMEM;
      break;
    }
    case LUA_GCRESTART: {
      g->GCthreshold = g->totalbytes;
      break;
    }
    case LUA_GCCOLLECT: {
      luaC_fullgc(L);
      break;
    }
    case LUA_GCCOUNT: {
      /* GC values are expressed in Kbytes: #bytes/2^10 */
      res = cast_int(g->totalbytes >> 10);
      break;
    }
    case LUA_GCCOUNTB: {
      res = cast_int(g->totalbytes & 0x3ff);
      break;
    }
    case LUA_GCSTEP: {
      lu_mem a = (cast(lu_mem, data) << 10);
      if (a <= g->totalbytes)
        g->GCthreshold = g->totalbytes - a;
      else
        g->GCthreshold = 0;
      while (g->GCthreshold <= g->totalbytes) {
        luaC_step(L);
        if (g->gcstate == GCSpause) {  /* end of cycle? */
          res = 1;  /* signal it */
          break;
        }
      }
      break;
    }
    case LUA_GCSETPAUSE: {
      res = g->gcpause;
      g->gcpause = data;
      break;
    }
    case LUA_GCSETSTEPMUL: {
      res = g->gcstepmul;
      g->gcstepmul = data;
      break;
    }
    default: res = -1;  /* invalid option */
  }
  lua_unlock(L);
  return res;
}



/*
** miscellaneous functions
*/


LUA_API int lua_error (lua_State *L) {
  lua_lock(L);
  api_checknelems(L, 1);
#if LUA_EXT_RESUMABLEVM
  luaD_throw(L, LUA_ERRRUN);
#else
  luaG_errormsg(L);
#endif /* LUA_EXT_RESUMABLEVM */
  lua_unlock(L);
  return 0;  /* to avoid warnings */
}


LUA_API int lua_next (lua_State *L, int idx) {
  StkId t;
  int more;
  lua_lock(L);
  t = index2adr(L, idx);
  api_check(L, ttistable(t));
  more = luaH_next(L, hvalue(t), L->top - 1);
  if (more) {
    api_incr_top(L);
  }
#if LUA_REFCOUNT
  else {  /* no more elements */
    L->top -= 1;  /* remove key */
    luarc_cleanvalue(L->top);
  }
#else
  else  /* no more elements */
    L->top -= 1;  /* remove key */
#endif /* LUA_REFCOUNT */
  lua_unlock(L);
  return more;
}


LUA_API void lua_concat (lua_State *L, int n) {
  lua_lock(L);
  api_checknelems(L, n);
  if (n >= 2) {
    luaC_checkGC(L);
#if LUA_EXT_RESUMABLEVM
    notresumable(L,
      luaV_concat(L, n, cast_int(L->top - L->base) - 1);
    )
#else
    luaV_concat(L, n, cast_int(L->top - L->base) - 1);
#endif /* LUA_EXT_RESUMABLEVM */
    L->top -= (n-1);
  }
  else if (n == 0) {  /* push empty string */
    setsvalue2s(L, L->top, luaS_newlstr(L, "", 0));
    api_incr_top(L);
  }
  /* else n == 1; nothing to do */
  lua_unlock(L);
}


LUA_API lua_Alloc lua_getallocf (lua_State *L, void **ud) {
  lua_Alloc f;
  lua_lock(L);
  if (ud) *ud = G(L)->ud;
  f = G(L)->frealloc;
  lua_unlock(L);
  return f;
}


LUA_API void lua_setallocf (lua_State *L, lua_Alloc f, void *ud) {
  lua_lock(L);
  G(L)->ud = ud;
  G(L)->frealloc = f;
  lua_unlock(L);
}


LUA_API void *lua_newuserdata (lua_State *L, size_t size) {
  Udata *u;
  lua_lock(L);
  luaC_checkGC(L);
  u = luaS_newudata(L, size, getcurrenv(L));
  setuvalue(L, L->top, u);
  api_incr_top(L);
  lua_unlock(L);
  return u + 1;
}




static const char *aux_upvalue (StkId fi, int n, TValue **val) {
  Closure *f;
  if (!ttisfunction(fi)) return NULL;
  f = clvalue(fi);
  if (f->c.isC) {
    if (!(1 <= n && n <= f->c.nupvalues)) return NULL;
    *val = &f->c.upvalue[n-1];
    return "";
  }
  else {
    Proto *p = f->l.p;
    if (!(1 <= n && n <= p->sizeupvalues)) return NULL;
    *val = f->l.upvals[n-1]->v;
    return getstr(p->upvalues[n-1]);
  }
}


LUA_API const char *lua_getupvalue (lua_State *L, int funcindex, int n) {
  const char *name;
  TValue *val;
  lua_lock(L);
  name = aux_upvalue(index2adr(L, funcindex), n, &val);
  if (name) {
    setobj2s(L, L->top, val);
    api_incr_top(L);
  }
  lua_unlock(L);
  return name;
}


LUA_API const char *lua_setupvalue (lua_State *L, int funcindex, int n) {
  const char *name;
  TValue *val;
  StkId fi;
  lua_lock(L);
  fi = index2adr(L, funcindex);
  api_checknelems(L, 1);
  name = aux_upvalue(fi, n, &val);
  if (name) {
    L->top--;
    setobj(L, val, L->top);
    luaC_barrier(L, clvalue(fi), L->top);
#if LUA_REFCOUNT
    luarc_cleanvalue(L->top);
#endif /* LUA_REFCOUNT */
  }
  lua_unlock(L);
  return name;
}

#if LUA_FASTREF_SUPPORT

/*
** {======================================================
** Reference system
** =======================================================
*/

LUA_API int lua_fastrefindex (lua_State *L, int idx) {
  const TValue* firstfree;
  TValue* value;
  TValue* newKey;
  lua_Integer ref;
  StkId to = &G(L)->l_refs;

  lua_lock(L);

  value = index2adr(L, idx);
  if (ttype(value) == LUA_TNIL) {
    lua_unlock(L);
    return LUA_FASTREFNIL;
  }

  /* get first free element */
  firstfree = luaH_getinthelper(hvalue(to), LUA_RIDX_FASTREF_FREELIST);
  lua_number2integer(ref, nvalue(firstfree));

  if (ref != 0) {  /* any free element? */
    /* remove it from list */
    const TValue* refValue = luaH_getinthelper(hvalue(to), (int)ref);
    setobj2t(L, (TValue*)firstfree, refValue);
  }
  else  /* no free elements */
    ref = (int)luaH_getn(hvalue(to)) + 1;  /* get a new reference */

  newKey = luaH_setinthelper(L, hvalue(to), (int)ref);
  if (idx <= LUA_FASTREFNIL) {
    if (idx == LUA_FASTREFNIL) {
      value = &G(L)->fastrefNilValue;
    } else {
      idx = -idx + LUA_FASTREFNIL - 1;
      value = (TValue*)luaH_getinthelper(hvalue(&G(L)->l_refs), idx);
    }
  } else {
    value = index2adr(L, idx);
  }
  setobj2t(L, newKey, value);

  luaC_barriert(L, hvalue(to), value);

  lua_unlock(L);
  return LUA_FASTREFNIL - 1 - (int)ref;
}


LUA_API int lua_fastref (lua_State *L) {
  int ref;
  lua_lock(L);
  ref = lua_fastrefindex(L, -1);
  L->top--;
  lua_unlock(L);
  return ref;
}


LUA_API void lua_fastunref (lua_State *L, int ref) {
  ref = -ref + LUA_FASTREFNIL - 1;
  if (ref >= 0) {
    TValue refObj;
    StkId to = &G(L)->l_refs;
    const TValue *firstfree = luaH_getinthelper(hvalue(to), LUA_RIDX_FASTREF_FREELIST);
    setobj2t(L, luaH_setinthelper(L, hvalue(to), ref), firstfree);
    setnvalue(&refObj, cast_num(ref));
    setobj2t(L, (TValue*)firstfree, &refObj);
  }
}


LUA_API void lua_getfastref (lua_State *L, int ref) {
  StkId o = &G(L)->l_refs;
  ref = -ref + LUA_FASTREFNIL - 1;
  lua_lock(L);
  setobj2s(L, L->top, luaH_getinthelper(hvalue(o), ref));
  api_incr_top(L);
  lua_unlock(L);
}


/* }====================================================== */

#endif /* LUA_FASTREF_SUPPORT */

NAMESPACE_LUA_END
