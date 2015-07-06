/*
** $Id: lvm.c,v 2.63.1.3 2007/12/28 15:32:23 roberto Exp $
** Lua virtual machine
** See Copyright Notice in lua.h
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define lvm_c
#define LUA_CORE

#include "lua.h"

#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lgc.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "ltm.h"
#include "lvm.h"

NAMESPACE_LUA_BEGIN

/* limit for table tag-method chains (to avoid loops) */
#define MAXTAGLOOP	100


#if LUA_REFCOUNT
const TValue *luaV_tonumber (lua_State *L, const TValue *obj, TValue *n) {
#else
const TValue *luaV_tonumber (const TValue *obj, TValue *n) {
#endif /* LUA_REFCOUNT */
  lua_Number num;
  if (ttisnumber(obj)) return obj;
  if (ttisstring(obj) && luaO_str2d(svalue(obj), &num)) {
    setnvalue(n, num);
    return n;
  }
#if LUA_WIDESTRING
  else if (ttiswstring(obj) && luaO_wstr2d(wsvalue(obj), &num)) {
    setnvalue(n, num);
    return n;
  }
#endif /* LUA_WIDESTRING */
  else
    return NULL;
}


int luaV_tostring (lua_State *L, StkId obj) {
  if (!ttisnumber(obj))
    return 0;
  else {
    char s[LUAI_MAXNUMBER2STR];
    lua_Number n = nvalue(obj);
    lua_number2str(s, n);
    setsvalue2s(L, obj, luaS_new(L, s));
    return 1;
  }
}


#if LUA_EXT_RESUMABLEVM
static StkId traceexec (lua_State *L, const Instruction *pc) {
#else
static void traceexec (lua_State *L, const Instruction *pc) {
#endif /* LUA_EXT_RESUMABLEVM */
  lu_byte mask = L->hookmask;
#if LUA_EXT_RESUMABLEVM
  const Instruction *oldpc = GETPC(L);
  SAVEPC(L, pc);
#else
  const Instruction *oldpc = L->savedpc;
  L->savedpc = pc;
#endif /* LUA_EXT_RESUMABLEVM */
  if ((mask & LUA_MASKCOUNT) && L->hookcount == 0) {
    resethookcount(L);
    luaD_callhook(L, LUA_HOOKCOUNT, -1);
  }
  if (mask & LUA_MASKLINE) {
    Proto *p = ci_func(L->ci)->l.p;
    int npc = pcRel(pc, p);
    int newline = getline(p, npc);
    /* call linehook when enter a new function, when jump back (loop),
       or when enter a new line */
    if (npc == 0 || pc <= oldpc || newline != getline(p, pcRel(oldpc, p)))
      luaD_callhook(L, LUA_HOOKLINE, newline);
  }
#if LUA_EXT_RESUMABLEVM
  return L->base;
#endif /* LUA_EXT_RESUMABLEVM */
}


static void callTMres (lua_State *L, StkId res, const TValue *f,
                        const TValue *p1, const TValue *p2) {
  ptrdiff_t result = savestack(L, res);
  setobj2s(L, L->top, f);  /* push function */
  setobj2s(L, L->top+1, p1);  /* 1st argument */
  setobj2s(L, L->top+2, p2);  /* 2nd argument */
  luaD_checkstack(L, 3);
  L->top += 3;
#if LUA_EXT_RESUMABLEVM
  luaD_call(L, L->top - 3, 1, 0);
#else
  luaD_call(L, L->top - 3, 1);
#endif /* LUA_EXT_RESUMABLEVM */
  res = restorestack(L, result);
  L->top--;
  setobjs2s(L, res, L->top);
}



static void callTM (lua_State *L, const TValue *f, const TValue *p1,
                    const TValue *p2, const TValue *p3) {
  setobj2s(L, L->top, f);  /* push function */
  setobj2s(L, L->top+1, p1);  /* 1st argument */
  setobj2s(L, L->top+2, p2);  /* 2nd argument */
  setobj2s(L, L->top+3, p3);  /* 3th argument */
  luaD_checkstack(L, 4);
  L->top += 4;
#if LUA_EXT_RESUMABLEVM
  luaD_call(L, L->top - 4, 0, 0);
#else
  luaD_call(L, L->top - 4, 0);
#endif /* LUA_EXT_RESUMABLEVM */
}


void luaV_gettable (lua_State *L, const TValue *t, TValue *key, StkId val) {
  int loop;
  for (loop = 0; loop < MAXTAGLOOP; loop++) {
    const TValue *tm;
    if (ttistable(t)) {  /* `t' is a table? */
      Table *h = hvalue(t);
      const TValue *res = luaH_get(h, key); /* do a primitive get */
      if (!ttisnil(res) ||  /* result is no nil? */
          (tm = fasttm(L, h->metatable, TM_INDEX)) == NULL) { /* or no TM? */
        setobj2s(L, val, res);
        return;
      }
      /* else will try the tag method */
    }
    else if (ttisnil(tm = luaT_gettmbyobj(L, t, TM_INDEX)))
      luaG_typeerror(L, t, "index");
    if (ttisfunction(tm)) {
      callTMres(L, val, tm, t, key);
      return;
    }
    t = tm;  /* else repeat with `tm' */
  }
  luaG_runerror(L, "loop in gettable");
}


void luaV_settable (lua_State *L, const TValue *t, TValue *key, StkId val) {
  int loop;
  TValue temp;
  for (loop = 0; loop < MAXTAGLOOP; loop++) {
    const TValue *tm;
    if (ttistable(t)) {  /* `t' is a table? */
      Table *h = hvalue(t);
      TValue *oldval = luaH_set(L, h, key); /* do a primitive set */
      if (!ttisnil(oldval) ||  /* result is no nil? */
          (tm = fasttm(L, h->metatable, TM_NEWINDEX)) == NULL) { /* or no TM? */
        setobj2t(L, oldval, val);
        luaC_barriert(L, h, val);
#if LUA_REFCOUNT
        if (ttisnil(val)) {
          Node* keyNode = luaH_getkey(h, key);
          if (keyNode) {
            luaH_removekey(L, h, keyNode);
          }
        }
#endif /* LUA_REFCOUNT */
        return;
      }
      /* else will try the tag method */
    }
    else if (ttisnil(tm = luaT_gettmbyobj(L, t, TM_NEWINDEX)))
      luaG_typeerror(L, t, "index");
    if (ttisfunction(tm)) {
      callTM(L, tm, t, key, val);
#if LUA_REFCOUNT
      {
        TValue *newval;
        if (ttistable(t)) {
          newval = luaH_set(L, hvalue(t), key);
          if (ttisnil(newval)) {
            Node* keyNode = luaH_getkey(hvalue(t), key);
            if (keyNode)
              luaH_removekey(L, hvalue(t), keyNode);
          }
        }
      }
#endif /* LUA_REFCOUNT */
      return;
    }
    /* else repeat with `tm' */
    setobj(L, &temp, tm);  /* avoid pointing inside table (may rehash) */
    t = &temp;
  }
  luaG_runerror(L, "loop in settable");
}


static int call_binTM (lua_State *L, const TValue *p1, const TValue *p2,
                       StkId res, TMS event) {
  const TValue *tm = luaT_gettmbyobj(L, p1, event);  /* try first operand */
  if (ttisnil(tm))
    tm = luaT_gettmbyobj(L, p2, event);  /* try second operand */
  if (ttisnil(tm)) return 0;
  callTMres(L, res, tm, p1, p2);
  return 1;
}


#if LUA_MUTATION_OPERATORS
static int call_compTM (lua_State *L, const TValue *p1, const TValue *p2,
                       StkId res, TMS event) {
  const TValue *tm = luaT_gettmbyobj(L, p1, event);  /* try first operand */
  if (ttisnil(tm)) return 0;
  callTMres(L, res, tm, p1, p2);
  return 1;
}
#endif /* LUA_MUTATION_OPERATORS */


static const TValue *get_compTM (lua_State *L, Table *mt1, Table *mt2,
                                  TMS event) {
  const TValue *tm1 = fasttm(L, mt1, event);
  const TValue *tm2;
  if (tm1 == NULL) return NULL;  /* no metamethod */
  if (mt1 == mt2) return tm1;  /* same metatables => same metamethods */
  tm2 = fasttm(L, mt2, event);
  if (tm2 == NULL) return NULL;  /* no metamethod */
  if (luaO_rawequalObj(tm1, tm2))  /* same metamethods? */
    return tm1;
  return NULL;
}


static int call_orderTM (lua_State *L, const TValue *p1, const TValue *p2,
                         TMS event) {
  const TValue *tm1 = luaT_gettmbyobj(L, p1, event);
  const TValue *tm2;
  if (ttisnil(tm1)) return -1;  /* no metamethod? */
  tm2 = luaT_gettmbyobj(L, p2, event);
  if (!luaO_rawequalObj(tm1, tm2))  /* different metamethods? */
    return -1;
  callTMres(L, L->top, tm1, p1, p2);
  return !l_isfalse(L->top);
}


static int l_strcmp (const TString *ls, const TString *rs) {
  const char *l = getstr(ls);
  size_t ll = ls->tsv.len;
  const char *r = getstr(rs);
  size_t lr = rs->tsv.len;
  for (;;) {
    int temp = strcoll(l, r);
    if (temp != 0) return temp;
    else {  /* strings are equal up to a `\0' */
      size_t len = strlen(l);  /* index of first `\0' in both strings */
      if (len == lr)  /* r is finished? */
        return (len == ll) ? 0 : 1;
      else if (len == ll)  /* l is finished? */
        return -1;  /* l is smaller than r (because r is not finished) */
      /* both strings longer than `len'; go on comparing (after the `\0') */
      len++;
      l += len; ll -= len; r += len; lr -= len;
    }
  }
}

#if LUA_WIDESTRING

size_t lua_WChar_len(const lua_WChar* str);
int lua_WChar_cmp(const lua_WChar* src, const lua_WChar* dest);

static int l_wstrcmp (const TString *ls, const TString *rs) {
  const lua_WChar *l = getwstr(ls);
  size_t ll = ls->tsv.len;
  const lua_WChar *r = getwstr(rs);
  size_t lr = rs->tsv.len;
  for (;;) {
    int temp = lua_WChar_cmp(l, r);
    if (temp != 0) return temp;
    else {  /* strings are equal up to a `\0' */
      size_t len = lua_WChar_len(l);  /* index of first `\0' in both strings */
      if (len == lr)  /* r is finished? */
        return (len == ll) ? 0 : 1;
      else if (len == ll)  /* l is finished? */
        return -1;  /* l is smaller than r (because r is not finished) */
      /* both strings longer than `len'; go on comparing (after the `\0') */
      len++;
      l += len; ll -= len; r += len; lr -= len;
    }
  }
}

#endif /* LUA_WIDESTRING */

int luaV_lessthan (lua_State *L, const TValue *l, const TValue *r) {
  int res;
  if (ttype(l) != ttype(r))
    return luaG_ordererror(L, l, r);
  else if (ttisnumber(l))
    return luai_numlt(nvalue(l), nvalue(r));
  else if (ttisstring(l))
    return l_strcmp(rawtsvalue(l), rawtsvalue(r)) < 0;
#if LUA_WIDESTRING
  else if (ttiswstring(l))
    return l_wstrcmp(rawtwsvalue(l), rawtwsvalue(r)) < 0;
#endif /* LUA_WIDESTRING */
  else if ((res = call_orderTM(L, l, r, TM_LT)) != -1)
    return res;
  return luaG_ordererror(L, l, r);
}


static int lessequal (lua_State *L, const TValue *l, const TValue *r) {
  int res;
  if (ttype(l) != ttype(r))
    return luaG_ordererror(L, l, r);
  else if (ttisnumber(l))
    return luai_numle(nvalue(l), nvalue(r));
  else if (ttisstring(l))
    return l_strcmp(rawtsvalue(l), rawtsvalue(r)) <= 0;
#if LUA_WIDESTRING
  else if (ttiswstring(l))
    return l_wstrcmp(rawtwsvalue(l), rawtwsvalue(r)) <= 0;
#endif /* LUA_WIDESTRING */
  else if ((res = call_orderTM(L, l, r, TM_LE)) != -1)  /* first try `le' */
    return res;
  else if ((res = call_orderTM(L, r, l, TM_LT)) != -1)  /* else try `lt' */
    return !res;
  return luaG_ordererror(L, l, r);
}


int luaV_equalval (lua_State *L, const TValue *t1, const TValue *t2) {
  const TValue *tm;
  lua_assert(ttype(t1) == ttype(t2));
  switch (ttype(t1)) {
    case LUA_TNIL: return 1;
    case LUA_TNUMBER: return luai_numeq(nvalue(t1), nvalue(t2));
    case LUA_TBOOLEAN: return bvalue(t1) == bvalue(t2);  /* true must be 1 !! */
    case LUA_TLIGHTUSERDATA: return pvalue(t1) == pvalue(t2);
    case LUA_TUSERDATA: {
      if (uvalue(t1) == uvalue(t2)) return 1;
      tm = get_compTM(L, uvalue(t1)->metatable, uvalue(t2)->metatable,
                         TM_EQ);
      break;  /* will try TM */
    }
    case LUA_TTABLE: {
      if (hvalue(t1) == hvalue(t2)) return 1;
      tm = get_compTM(L, hvalue(t1)->metatable, hvalue(t2)->metatable, TM_EQ);
      break;  /* will try TM */
    }
    default: return gcvalue(t1) == gcvalue(t2);
  }
  if (tm == NULL) return 0;  /* no TM? */
  callTMres(L, L->top, tm, t1, t2);  /* call TM */
  return !l_isfalse(L->top);
}

#if LUA_WIDESTRING

void luaV_concat (lua_State *L, int total, int last) {
  int useType = LUA_TSTRING;
  int i;
  StkId top = L->base + last + 1;

  for (i = 0; i < total; ++i)
  {
    if (ttype(top-1-i) == LUA_TSTRING || ttype(top-1-i) == LUA_TWSTRING)
    {
      useType = ttype(top-1-i);
      break;
    }
  }

  if (useType == LUA_TSTRING)
  {
    do {
      StkId top = L->base + last + 1;
      int n = 2;  /* number of elements handled in this pass (at least 2) */
      if (!tostring(L, top-2) || !tostring(L, top-1)) {
#if LUA_EXT_RESUMABLEVM
        setpvalue(L->top, (void *)(ptrdiff_t)(last - 1));  /* for luaV_resume */
        L->top++;
#endif /* LUA_EXT_RESUMABLEVM */
        if (!call_binTM(L, top-2, top-1, top-2, TM_CONCAT))
          luaG_concaterror(L, top-2, top-1);
#if LUA_EXT_RESUMABLEVM
        L->top--;
#endif /* LUA_EXT_RESUMABLEVM */
      } else if (tsvalue(top-1)->len > 0) {  /* if len=0, do nothing */
        /* at least two string values; get as many as possible */
        size_t tl = tsvalue(top-1)->len;
        char *buffer;
        int i;
        /* collect total length */
        for (n = 1; n < total && tostring(L, top-n-1); n++) {
          size_t l = tsvalue(top-n-1)->len;
          if (l >= MAX_SIZET - tl) luaG_runerror(L, "string length overflow");
          tl += l;
        }
        buffer = luaZ_openspace(L, &G(L)->buff, tl);
        tl = 0;
        for (i=n; i>0; i--) {  /* concat all strings */
          size_t l = tsvalue(top-i)->len;
          memcpy(buffer+tl, svalue(top-i), l);
          tl += l;
#if LUA_REFCOUNT
          luarc_cleanvalue(top-i);
#endif /* LUA_REFCOUNT */
        }
        setsvalue2s(L, top-n, luaS_newlstr(L, buffer, tl));
      }
      total -= n-1;  /* got `n' strings to create 1 new */
      last -= n-1;
    } while (total > 1);  /* repeat until only 1 result left */
  } else {
    do {
      StkId top = L->base + last + 1;
      int n = 2;  /* number of elements handled in this pass (at least 2) */
      if (!towstring(L, top-2) || !towstring(L, top-1)) {
#if LUA_EXT_RESUMABLEVM
        setpvalue(L->top, (void *)(ptrdiff_t)(last - 1));  /* for luaV_resume */
        L->top++;
#endif /* LUA_EXT_RESUMABLEVM */
        if (!call_binTM(L, top-2, top-1, top-2, TM_CONCAT))
          luaG_concaterror(L, top-2, top-1);
#if LUA_EXT_RESUMABLEVM
        L->top--;
#endif /* LUA_EXT_RESUMABLEVM */
      } else if (tsvalue(top-1)->len > 0) {  /* if len=0, do nothing */
        /* at least two string values; get as many as possible */
        size_t tl = tsvalue(top-1)->len;
        char *buffer;
        int i;
        /* collect total length */
        for (n = 1; n < total && towstring(L, top-n-1); n++) {
          size_t l = tsvalue(top-n-1)->len;
          if (l >= MAX_SIZET - tl) luaG_runerror(L, "string length overflow");
          tl += l;
        }
        buffer = luaZ_openspace(L, &G(L)->buff, tl*2);
        tl = 0;
        for (i=n; i>0; i--) {  /* concat all strings */
          size_t l = tsvalue(top-i)->len;
          memcpy(buffer+tl*2, wsvalue(top-i), l*2);
          tl += l;
#if LUA_REFCOUNT
          luarc_cleanvalue(top-i);
#endif /* LUA_REFCOUNT */
        }
        setwsvalue2s(L, top-n, luaS_newlwstr(L, (const lua_WChar*)buffer, tl));
      }
      total -= n-1;  /* got `n' strings to create 1 new */
      last -= n-1;
    } while (total > 1);  /* repeat until only 1 result left */
  }
}

#else

void luaV_concat (lua_State *L, int total, int last) {
  do {
    StkId top = L->base + last + 1;
    int n = 2;  /* number of elements handled in this pass (at least 2) */
    if (!(ttisstring(top-2) || ttisnumber(top-2)) || !tostring(L, top-1)) {
      if (!call_binTM(L, top-2, top-1, top-2, TM_CONCAT))
        luaG_concaterror(L, top-2, top-1);
    } else if (tsvalue(top-1)->len == 0)  /* second op is empty? */
      (void)tostring(L, top - 2);  /* result is first op (as string) */
    else {
      /* at least two string values; get as many as possible */
      size_t tl = tsvalue(top-1)->len;
      char *buffer;
      int i;
      /* collect total length */
      for (n = 1; n < total && tostring(L, top-n-1); n++) {
        size_t l = tsvalue(top-n-1)->len;
        if (l >= MAX_SIZET - tl) luaG_runerror(L, "string length overflow");
        tl += l;
      }
      buffer = luaZ_openspace(L, &G(L)->buff, tl);
      tl = 0;
      for (i=n; i>0; i--) {  /* concat all strings */
        size_t l = tsvalue(top-i)->len;
        memcpy(buffer+tl, svalue(top-i), l);
        tl += l;
#if LUA_REFCOUNT
        luarc_cleanvalue(top-i);
#endif /* LUA_REFCOUNT */
      }
      setsvalue2s(L, top-n, luaS_newlstr(L, buffer, tl));
    }
    total -= n-1;  /* got `n' strings to create 1 new */
    last -= n-1;
  } while (total > 1);  /* repeat until only 1 result left */
}

#endif /* LUA_WIDESTRING */

static void Arith (lua_State *L, StkId ra, const TValue *rb,
                   const TValue *rc, TMS op) {
  TValue tempb, tempc;
  const TValue *b, *c;
#if LUA_REFCOUNT
  luarc_newvalue(&tempb);
  luarc_newvalue(&tempc);
  if ((b = luaV_tonumber(L, rb, &tempb)) != NULL &&
      (c = luaV_tonumber(L, rc, &tempc)) != NULL) {
#else
  if ((b = luaV_tonumber(rb, &tempb)) != NULL &&
      (c = luaV_tonumber(rc, &tempc)) != NULL) {
#endif /* LUA_REFCOUNT */
    lua_Number nb = nvalue(b), nc = nvalue(c);
#if LUA_REFCOUNT
    luarc_cleanvalue(&tempb);
    luarc_cleanvalue(&tempc);
#endif /* LUA_REFCOUNT */
    switch (op) {
      case TM_ADD: setnvalue(ra, luai_numadd(nb, nc)); break;
      case TM_SUB: setnvalue(ra, luai_numsub(nb, nc)); break;
      case TM_MUL: setnvalue(ra, luai_nummul(nb, nc)); break;
      case TM_DIV: setnvalue(ra, luai_numdiv(nb, nc)); break;
      case TM_MOD: setnvalue(ra, luai_nummod(nb, nc)); break;
      case TM_POW: setnvalue(ra, luai_numpow(nb, nc)); break;
      case TM_UNM: setnvalue(ra, luai_numunm(nb)); break;
      default: lua_assert(0); break;
    }
  }
#if LUA_REFCOUNT
  else if (!call_binTM(L, rb, rc, ra, op)) {
    luarc_cleanvalue(&tempb);
    luarc_cleanvalue(&tempc);
    luaG_aritherror(L, rb, rc);
  }
#else
  else if (!call_binTM(L, rb, rc, ra, op))
    luaG_aritherror(L, rb, rc);
#endif /* LUA_REFCOUNT */
}


#if LUA_MUTATION_OPERATORS
static void Compound (lua_State *L, StkId ra, const TValue *rb, TMS op) {
  TValue tempa;
  TValue tempb;
  const TValue *a, *b;
  if ((a = luaV_tonumber(ra, &tempa)) != NULL &&
      (b = luaV_tonumber(rb, &tempb)) != NULL) {
    lua_Number na = nvalue(a), nb = nvalue(b);
    switch (op) {
      case TM_ADD_EQ: setnvalue(ra, luai_numadd(na, nb)); break;
      case TM_SUB_EQ: setnvalue(ra, luai_numsub(na, nb)); break;
      case TM_MUL_EQ: setnvalue(ra, luai_nummul(na, nb)); break;
      case TM_DIV_EQ: setnvalue(ra, luai_numdiv(na, nb)); break;
      case TM_MOD_EQ: setnvalue(ra, luai_nummod(na, nb)); break;
      case TM_POW_EQ: setnvalue(ra, luai_numpow(na, nb)); break;
      default: lua_assert(0); break;
    }
  }
  else if (!call_compTM(L, ra, rb, ra, op))
    luaG_aritherror(L, ra, rb);
}
#endif /* LUA_MUTATION_OPERATORS */




/*
** some macros for common tasks in `luaV_execute'
*/

#define runtime_check(L, c)	{ if (!(c)) break; }

#define RA(i)	(base+GETARG_A(i))
/* to be used after possible stack reallocation */
#define RB(i)	check_exp(getBMode(GET_OPCODE(i)) == OpArgR, base+GETARG_B(i))
#define RC(i)	check_exp(getCMode(GET_OPCODE(i)) == OpArgR, base+GETARG_C(i))
#define RKB(i)	check_exp(getBMode(GET_OPCODE(i)) == OpArgK, \
	ISK(GETARG_B(i)) ? k+INDEXK(GETARG_B(i)) : base+GETARG_B(i))
#define RKC(i)	check_exp(getCMode(GET_OPCODE(i)) == OpArgK, \
	ISK(GETARG_C(i)) ? k+INDEXK(GETARG_C(i)) : base+GETARG_C(i))
#define KBx(i)	check_exp(getBMode(GET_OPCODE(i)) == OpArgK, k+GETARG_Bx(i))


#define dojump(L,pc,i)	{(pc) += (i); luai_threadyield(L);}


#if LUA_EXT_RESUMABLEVM
#define Protect(x)	{ SAVEPC(L, pc); {x;}; base = L->base; }
#else
#define Protect(x)	{ L->savedpc = pc; {x;}; base = L->base; }
#endif /* LUA_EXT_RESUMABLEVM */


#define arith_op(op,tm) { \
        TValue *rb = RKB(i); \
        TValue *rc = RKC(i); \
        if (ttisnumber(rb) && ttisnumber(rc)) { \
          lua_Number nb = nvalue(rb), nc = nvalue(rc); \
          setnvalue(ra, op(nb, nc)); \
        } \
        else \
          Protect(Arith(L, ra, rb, rc, tm)); \
      }

#if LUA_MUTATION_OPERATORS
#define compound_op(op,tm) { \
        TValue *rb = RKB(i); \
        if (ttisnumber(ra) && ttisnumber(rb)) { \
          lua_Number na = nvalue(ra), nb = nvalue(rb); \
          setnvalue(ra, op(na, nb)); \
        } \
        else \
          Protect(Compound(L, ra, rb, tm)); \
      }
#endif /* LUA_MUTATION_OPERATORS */

#if LUA_BITFIELD_OPS

#define bit_op(op) { \
        TValue *rb = RKB(i); \
        TValue *rc = RKC(i); \
        if (ttisnumber(rb) && ttisnumber(rc)) { \
          unsigned int nb = (unsigned int)nvalue(rb), nc = (unsigned int)nvalue(rc); \
          setnvalue(ra, nb op nc); \
        } \
        else \
          luaG_aritherror(L, rb, rc); \
      }

#endif /* LUA_BITFIELD_OPS */



#if LUA_EXT_RESUMABLEVM
int luaV_execute (lua_State *L) {
#else
void luaV_execute (lua_State *L, int nexeccalls) {
#endif /* LUA_EXT_RESUMABLEVM */
  LClosure *cl;
  StkId base;
  TValue *k;
  const Instruction *pc;
#if LUA_EXT_RESUMABLEVM
  int nexeccalls = 1;
#endif /* LUA_EXT_RESUMABLEVM */
 reentry:  /* entry point */
  lua_assert(isLua(L->ci));
#if LUA_EXT_RESUMABLEVM
  pc = GETPC(L);
#else
  pc = L->savedpc;
#endif /* LUA_EXT_RESUMABLEVM */
  cl = &clvalue(L->ci->func)->l;
  base = L->base;
  k = cl->p->k;
  /* main loop of interpreter */
  for (;;) {
    const Instruction i = *pc++;
    StkId ra;
#if LUA_EXT_RESUMABLEVM
    if ((L->hookmask & (LUA_MASKLINE | LUA_MASKCOUNT)) &&
        (--L->hookcount == 0 || L->hookmask & LUA_MASKLINE))
      base = traceexec(L, pc);
#else
    if ((L->hookmask & (LUA_MASKLINE | LUA_MASKCOUNT)) &&
        (--L->hookcount == 0 || L->hookmask & LUA_MASKLINE)) {
      traceexec(L, pc);
      if (L->status == LUA_YIELD) {  /* did hook yield? */
        L->savedpc = pc - 1;
        return;
      }
      base = L->base;
    }
#endif /* LUA_EXT_RESUMABLEVM */
    /* warning!! several calls may realloc the stack and invalidate `ra' */
    ra = RA(i);
    lua_assert(base == L->base && L->base == L->ci->base);
    lua_assert(base <= L->top && L->top <= L->stack + L->stacksize);
    lua_assert(L->top == L->ci->top || luaG_checkopenop(i));
    switch (GET_OPCODE(i)) {
      case OP_MOVE: {
        setobjs2s(L, ra, RB(i));
        continue;
      }
      case OP_LOADK: {
        setobj2s(L, ra, KBx(i));
        continue;
      }
      case OP_LOADBOOL: {
        setbvalue(ra, GETARG_B(i));
        if (GETARG_C(i)) pc++;  /* skip next instruction (if C) */
        continue;
      }
      case OP_LOADNIL: {
        TValue *rb = RB(i);
        do {
          setnilvalue(rb--);
        } while (rb >= ra);
        continue;
      }
      case OP_GETUPVAL: {
        int b = GETARG_B(i);
        setobj2s(L, ra, cl->upvals[b]->v);
        continue;
      }
      case OP_GETGLOBAL: {
        TValue g;
        TValue *rb = KBx(i);
#if LUA_REFCOUNT
        sethvalue2n(L, &g, cl->env);
#else
        sethvalue(L, &g, cl->env);
#endif /* LUA_REFCOUNT */
        lua_assert(ttisstring(rb));
        Protect(luaV_gettable(L, &g, rb, ra));
#if LUA_REFCOUNT
		setnilvalue(&g);
#endif /* LUA_REFCOUNT */
        continue;
      }
      case OP_GETTABLE: {
        Protect(luaV_gettable(L, RB(i), RKC(i), ra));
        continue;
      }
      case OP_SETGLOBAL: {
        TValue g;
#if LUA_REFCOUNT
        sethvalue2n(L, &g, cl->env);
#else
        sethvalue(L, &g, cl->env);
#endif /* LUA_REFCOUNT */
        lua_assert(ttisstring(KBx(i)));
        Protect(luaV_settable(L, &g, KBx(i), ra));
#if LUA_REFCOUNT
		setnilvalue(&g);
#endif /* LUA_REFCOUNT */
        continue;
      }
      case OP_SETUPVAL: {
        UpVal *uv = cl->upvals[GETARG_B(i)];
        setobj(L, uv->v, ra);
        luaC_barrier(L, uv, ra);
        continue;
      }
      case OP_SETTABLE: {
        Protect(luaV_settable(L, ra, RKB(i), RKC(i)));
        continue;
      }
      case OP_NEWTABLE: {
        int b = GETARG_B(i);
        int c = GETARG_C(i);
        sethvalue(L, ra, luaH_new(L, luaO_fb2int(b), luaO_fb2int(c)));
        Protect(luaC_checkGC(L));
        continue;
      }
      case OP_SELF: {
        StkId rb = RB(i);
        setobjs2s(L, ra+1, rb);
        Protect(luaV_gettable(L, rb, RKC(i), ra));
        continue;
      }
      case OP_ADD: {
        arith_op(luai_numadd, TM_ADD);
        continue;
      }
      case OP_SUB: {
        arith_op(luai_numsub, TM_SUB);
        continue;
      }
      case OP_MUL: {
        arith_op(luai_nummul, TM_MUL);
        continue;
      }
      case OP_DIV: {
        arith_op(luai_numdiv, TM_DIV);
        continue;
      }
      case OP_MOD: {
        arith_op(luai_nummod, TM_MOD);
        continue;
      }
      case OP_POW: {
        arith_op(luai_numpow, TM_POW);
        continue;
      }
      case OP_UNM: {
        TValue *rb = RB(i);
        if (ttisnumber(rb)) {
          lua_Number nb = nvalue(rb);
          setnvalue(ra, luai_numunm(nb));
        }
        else {
          Protect(Arith(L, ra, rb, rb, TM_UNM));
        }
        continue;
      }
      case OP_NOT: {
        int res = l_isfalse(RB(i));  /* next assignment may change this value */
        setbvalue(ra, res);
        continue;
      }
#if LUA_BITFIELD_OPS
      case OP_BAND: {
        bit_op(&);
        continue;
      }
      case OP_BOR: {
        bit_op(|);
        continue;
      }
      case OP_BXOR: {
        bit_op(^);
        continue;
      }
      case OP_BSHL: {
        bit_op(<<);
        continue;
      }
      case OP_BSHR: {
        bit_op(>>);
        continue;
      }
#endif /* LUA_BITFIELD_OPS */
      case OP_LEN: {
        const TValue *rb = RB(i);
        switch (ttype(rb)) {
          case LUA_TTABLE: {
            setnvalue(ra, cast_num(luaH_getn(hvalue(rb))));
            break;
          }
          case LUA_TSTRING: {
            setnvalue(ra, cast_num(tsvalue(rb)->len));
            break;
          }
#if LUA_WIDESTRING
          case LUA_TWSTRING: {
            setnvalue(ra, cast_num(tsvalue(rb)->len));
            break;
          }
#endif /* LUA_WIDESTRING */
          default: {  /* try metamethod */
            Protect(
              if (!call_binTM(L, rb, luaO_nilobject, ra, TM_LEN))
                luaG_typeerror(L, rb, "get length of");
            )
          }
        }
        continue;
      }
      case OP_CONCAT: {
        int b = GETARG_B(i);
        int c = GETARG_C(i);
        Protect(luaV_concat(L, c-b+1, c); luaC_checkGC(L));
        setobjs2s(L, RA(i), base+b);
        continue;
      }
      case OP_JMP: {
        dojump(L, pc, GETARG_sBx(i));
        continue;
      }
      case OP_EQ: {
        TValue *rb = RKB(i);
        TValue *rc = RKC(i);
        Protect(
          if (equalobj(L, rb, rc) == GETARG_A(i))
            dojump(L, pc, GETARG_sBx(*pc));
        )
        pc++;
        continue;
      }
      case OP_LT: {
        Protect(
          if (luaV_lessthan(L, RKB(i), RKC(i)) == GETARG_A(i))
            dojump(L, pc, GETARG_sBx(*pc));
        )
        pc++;
        continue;
      }
      case OP_LE: {
        Protect(
          if (lessequal(L, RKB(i), RKC(i)) == GETARG_A(i))
            dojump(L, pc, GETARG_sBx(*pc));
        )
        pc++;
        continue;
      }
      case OP_TEST: {
        if (l_isfalse(ra) != GETARG_C(i))
          dojump(L, pc, GETARG_sBx(*pc));
        pc++;
        continue;
      }
      case OP_TESTSET: {
        TValue *rb = RB(i);
        if (l_isfalse(rb) != GETARG_C(i)) {
          setobjs2s(L, ra, rb);
          dojump(L, pc, GETARG_sBx(*pc));
        }
        pc++;
        continue;
      }
      case OP_CALL: {
        int b = GETARG_B(i);
        int nresults = GETARG_C(i) - 1;
        if (b != 0) L->top = ra+b;  /* else previous instruction set top */
#if LUA_EXT_RESUMABLEVM
        SAVEPC(L, pc);
#else
        L->savedpc = pc;
#endif /* LUA_EXT_RESUMABLEVM */
        switch (luaD_precall(L, ra, nresults)) {
          case PCRLUA: {
            nexeccalls++;
            goto reentry;  /* restart luaV_execute over new Lua function */
          }
          case PCRC: {
            /* it was a C function (`precall' called it); adjust results */
            if (nresults >= 0) L->top = L->ci->top;
            base = L->base;
            continue;
          }
          default: {
#if LUA_EXT_RESUMABLEVM
            return LUA_YIELD;
#else
            return;  /* yield */
#endif /* LUA_EXT_RESUMABLEVM */
          }
        }
      }
      case OP_TAILCALL: {
        int b = GETARG_B(i);
        if (b != 0) L->top = ra+b;  /* else previous instruction set top */
#if LUA_EXT_RESUMABLEVM
        SAVEPC(L, pc);
#else
        L->savedpc = pc;
#endif /* LUA_EXT_RESUMABLEVM */
        lua_assert(GETARG_C(i) - 1 == LUA_MULTRET);
        switch (luaD_precall(L, ra, LUA_MULTRET)) {
          case PCRLUA: {
            /* tail call: put new frame in place of previous one */
            CallInfo *ci = L->ci - 1;  /* previous frame */
            int aux;
            StkId func = ci->func;
            StkId pfunc = (ci+1)->func;  /* previous function index */
            if (L->openupval) luaF_close(L, ci->base);
            L->base = ci->base = ci->func + ((ci+1)->base - pfunc);
            for (aux = 0; pfunc+aux < L->top; aux++)  /* move frame down */
              setobjs2s(L, func+aux, pfunc+aux);
            ci->top = L->top = func+aux;  /* correct top */
            lua_assert(L->top == L->base + clvalue(func)->l.p->maxstacksize);
#if LUA_EXT_RESUMABLEVM
            ci->ctx = L->ctx;
#else
            ci->savedpc = L->savedpc;
#endif /* LUA_EXT_RESUMABLEVM */
            ci->tailcalls++;  /* one more call lost */
            L->ci--;  /* remove new frame */
            goto reentry;
          }
          case PCRC: {  /* it was a C function (`precall' called it) */
            base = L->base;
            continue;
          }
          default: {
#if LUA_EXT_RESUMABLEVM
            return LUA_YIELD;
#else
            return;  /* yield */
#endif /* LUA_EXT_RESUMABLEVM */
          }
        }
      }
      case OP_RETURN: {
        int b = GETARG_B(i);
#if LUA_REFCOUNT
        StkId origTop = L->top;
		StkId oldTop;
#endif /* LUA_REFCOUNT */
        if (b != 0) L->top = ra+b-1;
        if (L->openupval) luaF_close(L, base);
#if LUA_EXT_RESUMABLEVM
        SAVEPC(L, pc);
#else
        L->savedpc = pc;
#endif /* LUA_EXT_RESUMABLEVM */
        b = luaD_poscall(L, ra);
#if LUA_REFCOUNT
        oldTop = L->top;
        L->top = origTop;
        luarc_cleanarray(ra+b, origTop);
        L->top = oldTop;
#endif /* LUA_REFCOUNT */
        if (--nexeccalls == 0)  /* was previous function running `here'? */
#if LUA_EXT_RESUMABLEVM
          return 0;  /* no: return */
#else
          return;  /* no: return */
#endif /* LUA_EXT_RESUMABLEVM */
        else {  /* yes: continue its execution */
          if (b) L->top = L->ci->top;
#if LUA_EXT_RESUMABLEVM
          lua_assert(isLua(L->ci) && GET_OPCODE(*(GETPC(L)-1)) == OP_CALL);
#else
          lua_assert(isLua(L->ci));
          lua_assert(GET_OPCODE(*((L->ci)->savedpc - 1)) == OP_CALL);
#endif /* LUA_EXT_RESUMABLEVM */
          goto reentry;
        }
      }
      case OP_FORLOOP: {
        lua_Number step = nvalue(ra+2);
        lua_Number idx = luai_numadd(nvalue(ra), step); /* increment index */
        lua_Number limit = nvalue(ra+1);
        if (luai_numlt(0, step) ? luai_numle(idx, limit)
                                : luai_numle(limit, idx)) {
          dojump(L, pc, GETARG_sBx(i));  /* jump back */
          setnvalue(ra, idx);  /* update internal index... */
          setnvalue(ra+3, idx);  /* ...and external index */
        }
        continue;
      }
      case OP_FORPREP: {
        const TValue *init = ra;
        const TValue *plimit = ra+1;
        const TValue *pstep = ra+2;
#if LUA_EXT_RESUMABLEVM
        SAVEPC(L, pc);  /* next steps may throw errors */
#else
        L->savedpc = pc;  /* next steps may throw errors */
#endif /* LUA_EXT_RESUMABLEVM */
        if (!tonumber(init, ra))
          luaG_runerror(L, LUA_QL("for") " initial value must be a number");
        else if (!tonumber(plimit, ra+1))
          luaG_runerror(L, LUA_QL("for") " limit must be a number");
        else if (!tonumber(pstep, ra+2))
          luaG_runerror(L, LUA_QL("for") " step must be a number");
        setnvalue(ra, luai_numsub(nvalue(ra), nvalue(pstep)));
        dojump(L, pc, GETARG_sBx(i));
        continue;
      }
      case OP_TFORLOOP: {
        StkId cb = ra + 3;  /* call base */
        setobjs2s(L, cb+2, ra+2);
        setobjs2s(L, cb+1, ra+1);
        setobjs2s(L, cb, ra);
        L->top = cb+3;  /* func. + 2 args (state and index) */
#if LUA_EXT_RESUMABLEVM
        Protect(luaD_call(L, cb, GETARG_C(i), 0));
#else
        Protect(luaD_call(L, cb, GETARG_C(i)));
#endif /* LUA_EXT_RESUMABLEVM */
        L->top = L->ci->top;
        cb = RA(i) + 3;  /* previous call may change the stack */
        if (!ttisnil(cb)) {  /* continue loop? */
          setobjs2s(L, cb-1, cb);  /* save control variable */
          dojump(L, pc, GETARG_sBx(*pc));  /* jump back */
        }
        pc++;
        continue;
      }
      case OP_SETLIST: {
        int n = GETARG_B(i);
        int c = GETARG_C(i);
        int last;
        Table *h;
        if (n == 0) {
          n = cast_int(L->top - ra) - 1;
          L->top = L->ci->top;
        }
        if (c == 0) c = cast_int(*pc++);
        runtime_check(L, ttistable(ra));
        h = hvalue(ra);
        last = ((c-1)*LFIELDS_PER_FLUSH) + n;
        if (last > h->sizearray)  /* needs more space? */
          luaH_resizearray(L, h, last);  /* pre-alloc it at once */
        for (; n > 0; n--) {
          TValue *val = ra+n;
          setobj2t(L, luaH_setnum(L, h, last--), val);
          luaC_barriert(L, h, val);
#if LUA_REFCOUNT
          setnilvalue(val);
#endif /* LUA_REFCOUNT */
        }
        continue;
      }
      case OP_CLOSE: {
        luaF_close(L, ra);
        continue;
      }
      case OP_CLOSURE: {
        Proto *p;
        Closure *ncl;
        int nup, j;
        p = cl->p->p[GETARG_Bx(i)];
        nup = p->nups;
        ncl = luaF_newLclosure(L, nup, cl->env);
        ncl->l.p = p;
#if LUA_REFCOUNT
        luarc_addrefproto(ncl->l.p);
#endif /* LUA_REFCOUNT */
        for (j=0; j<nup; j++, pc++) {
          if (GET_OPCODE(*pc) == OP_GETUPVAL)
            ncl->l.upvals[j] = cl->upvals[GETARG_B(*pc)];
          else {
            lua_assert(GET_OPCODE(*pc) == OP_MOVE);
            ncl->l.upvals[j] = luaF_findupval(L, base + GETARG_B(*pc));
          }
#if LUA_REFCOUNT
          luarc_addrefupval(ncl->l.upvals[j]);
          luarc_addref(&ncl->l.upvals[j]->u.value);
#endif /* LUA_REFCOUNT */
        }
        setclvalue(L, ra, ncl);
        Protect(luaC_checkGC(L));
        continue;
      }
      case OP_VARARG: {
        int b = GETARG_B(i) - 1;
        int j;
        CallInfo *ci = L->ci;
        int n = cast_int(ci->base - ci->func) - cl->p->numparams - 1;
        if (b == LUA_MULTRET) {
          Protect(luaD_checkstack(L, n));
          ra = RA(i);  /* previous call may change the stack */
          b = n;
          L->top = ra + n;
        }
        for (j = 0; j < b; j++) {
          if (j < n) {
            setobjs2s(L, ra + j, ci->base - n + j);
          }
          else {
            setnilvalue(ra + j);
          }
        }
        continue;
      }
#if LUA_MUTATION_OPERATORS
      case OP_ADD_EQ: {
        compound_op(luai_numadd, TM_ADD_EQ);
        continue;
      }
      case OP_SUB_EQ: {
        compound_op(luai_numsub, TM_SUB_EQ);
        continue;
      }
      case OP_MUL_EQ: {
        compound_op(luai_nummul, TM_MUL_EQ);
        continue;
      }
      case OP_DIV_EQ: {
        compound_op(luai_numdiv, TM_DIV_EQ);
        continue;
      }
      case OP_MOD_EQ: {
        compound_op(luai_nummod, TM_MOD_EQ);
        continue;
      }
      case OP_POW_EQ: {
        compound_op(luai_numpow, TM_POW_EQ);
        continue;
      }
#endif /* LUA_MUTATION_OPERATORS */
    }
  }
}

#if LUA_EXT_RESUMABLEVM

void luaV_resume (lua_State *L) {
  const Instruction *pc = GETPC(L);
  const Instruction i = *(pc - 1);
  switch (GET_OPCODE(i)) {  /* finish opcodes */
  case OP_CALL:
    if (i & MASK1(SIZE_C,POS_C)) L->top = L->ci->top;
    break;
  case OP_SETGLOBAL: case OP_SETTABLE: case OP_TAILCALL:
    break;  /* ok, but nothing to do */
  case OP_GETGLOBAL: case OP_GETTABLE: case OP_SELF: case OP_ADD: case OP_SUB:
  case OP_MUL: case OP_DIV: case OP_MOD: case OP_POW: case OP_UNM: case OP_LEN:
    L->top--;
    setobjs2s(L, L->base + GETARG_A(i), L->top);
    break;
  case OP_LT: case OP_LE: case OP_EQ:
    L->top--;
    if (!l_isfalse(L->top) != GETARG_A(i)) pc++;
    else dojump(L, pc, GETARG_sBx(*pc) + 1);
    SAVEPC(L, pc);
    break;
  case OP_TFORLOOP: {
    StkId cb;  /* call base */
    L->top = L->ci->top;
    cb = L->base + GETARG_A(i) + 3;
    if (ttisnil(cb))  /* break loop? */
      pc++;  /* skip jump (break loop) */
    else {
      setobjs2s(L, cb-1, cb);  /* save control variable */
      dojump(L, pc, GETARG_sBx(*pc) + 1);  /* jump back */
    }
    SAVEPC(L, pc);
    break;
  }
  case OP_CONCAT: {
    int b = GETARG_B(i);
    int c;
    L->top -= 2;
    c = (int)(ptrdiff_t)pvalue(L->top);
    setobjs2s(L, L->base + c, L->top + 1);
    if (c > b) luaV_concat(L, c-b+1, c);
    luaC_checkGC(L);  /***/
    setobjs2s(L, L->base + GETARG_A(i), L->base + b);
    break;
  }
  default:
    luaG_runerror(L, "return to non-resumable opcode %d", GET_OPCODE(i));
    break;
  }
  lua_assert(L->top == L->ci->top ||
             GET_OPCODE(i) == OP_CALL || GET_OPCODE(i) == OP_TAILCALL);
}

#endif /* LUA_EXT_RESUMABLEVM */

NAMESPACE_LUA_END
