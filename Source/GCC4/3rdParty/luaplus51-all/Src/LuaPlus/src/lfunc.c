/*
** $Id: lfunc.c,v 2.12.1.2 2007/12/28 14:58:43 roberto Exp $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/


#include <stddef.h>

#define lfunc_c
#define LUA_CORE

#include "lua.h"

#include "lfunc.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"


NAMESPACE_LUA_BEGIN

Closure *luaF_newCclosure (lua_State *L, int nelems, Table *e) {
#if LUA_MEMORY_STATS
  Closure *c;
  luaM_setname(L, "lua.cclosure");
  c = cast(Closure *, luaM_malloc(L, sizeCclosure(nelems)));
#else
  Closure *c = cast(Closure *, luaM_malloc(L, sizeCclosure(nelems)));
#endif /* LUA_MEMORY_STATS */
#if LUA_REFCOUNT
  c->c.ref = 0;
#endif /* LUA_REFCOUNT */
  luaC_link(L, obj2gco(c), LUA_TFUNCTION);
  c->c.isC = 1;
  c->c.env = e;
#if LUA_REFCOUNT
  luarc_addreftable(e);
  c->c.gclist = NULL;
#endif /* LUA_REFCOUNT */
  c->c.nupvalues = cast_byte(nelems);
#if LUA_MEMORY_STATS
  luaM_setname(L, 0);
#endif /* LUA_MEMORY_STATS */
  return c;
}


Closure *luaF_newLclosure (lua_State *L, int nelems, Table *e) {
#if LUA_MEMORY_STATS
  Closure *c;
  luaM_setname(L, "lua.lclosure");
  c = cast(Closure *, luaM_malloc(L, sizeLclosure(nelems)));
#else
  Closure *c = cast(Closure *, luaM_malloc(L, sizeLclosure(nelems)));
#endif /* LUA_MEMORY_STATS */
#if LUA_REFCOUNT
  c->l.ref = 0;
#endif /* LUA_REFCOUNT */
  luaC_link(L, obj2gco(c), LUA_TFUNCTION);
  c->l.isC = 0;
  c->l.env = e;
#if LUA_REFCOUNT
  luarc_addreftable(e);
  c->l.gclist = NULL;
#endif /* LUA_REFCOUNT */
  c->l.nupvalues = cast_byte(nelems);
  while (nelems--) c->l.upvals[nelems] = NULL;
#if LUA_MEMORY_STATS
  luaM_setname(L, 0);
#endif /* LUA_MEMORY_STATS */
  return c;
}


UpVal *luaF_newupval (lua_State *L) {
#if LUA_MEMORY_STATS
  UpVal *uv;
  luaM_setname(L, "lua.upval");
  uv = luaM_new(L, UpVal);
#else
  UpVal *uv = luaM_new(L, UpVal);
#endif /* LUA_MEMORY_STATS */
  luaC_link(L, obj2gco(uv), LUA_TUPVAL);
  uv->v = &uv->u.value;
#if LUA_REFCOUNT
  luarc_newvalue(uv->v);
#else
  setnilvalue(uv->v);
#endif /* LUA_REFCOUNT */
#if LUA_MEMORY_STATS
  luaM_setname(L, 0);
#endif /* LUA_MEMORY_STATS */
  return uv;
}


UpVal *luaF_findupval (lua_State *L, StkId level) {
  global_State *g = G(L);
  GCObject **pp = &L->openupval;
  UpVal *p;
  UpVal *uv;
  while (*pp != NULL && (p = ngcotouv(*pp))->v >= level) {
    lua_assert(p->v != &p->u.value);
    if (p->v == level) {  /* found a corresponding upvalue? */
      if (isdead(g, obj2gco(p)))  /* is it dead? */
        changewhite(obj2gco(p));  /* ressurect it */
      return p;
    }
    pp = &p->next;
  }
#if LUA_MEMORY_STATS
  luaM_setname(L, "lua.upval");
#endif /* LUA_MEMORY_STATS */
  uv = luaM_new(L, UpVal);  /* not found: create a new one */
  uv->tt = LUA_TUPVAL;
  uv->marked = luaC_white(g);
  uv->v = level;  /* current value lives in the stack */
  uv->next = *pp;  /* chain it in the proper position */
#if LUA_REFCOUNT
  if (uv->next) {
    uv->prev = uv->next->gch.prev;
    uv->next->gch.prev = (GCObject*)uv;
  } else {
    uv->prev = NULL;
  }
  luarc_newvalue(&uv->u.value);
  uv->ref = 1;
#endif /* LUA_REFCOUNT */
  *pp = obj2gco(uv);
  uv->u.l.prev = &g->uvhead;  /* double link it in `uvhead' list */
  uv->u.l.next = g->uvhead.u.l.next;
  uv->u.l.next->u.l.prev = uv;
  g->uvhead.u.l.next = uv;
  lua_assert(uv->u.l.next->u.l.prev == uv && uv->u.l.prev->u.l.next == uv);
#if LUA_MEMORY_STATS
  luaM_setname(L, 0);
#endif /* LUA_MEMORY_STATS */
  return uv;
}


static void unlinkupval (UpVal *uv) {
  lua_assert(uv->u.l.next->u.l.prev == uv && uv->u.l.prev->u.l.next == uv);
  uv->u.l.next->u.l.prev = uv->u.l.prev;  /* remove from `uvhead' list */
  uv->u.l.prev->u.l.next = uv->u.l.next;
}


void luaF_freeupval (lua_State *L, UpVal *uv) {
  if (uv->v != &uv->u.value)  /* is it open? */
    unlinkupval(uv);  /* remove from open list */
  luaM_free(L, uv);  /* free upvalue */
}


void luaF_close (lua_State *L, StkId level) {
  UpVal *uv;
  global_State *g = G(L);
  while (L->openupval != NULL && (uv = ngcotouv(L->openupval))->v >= level) {
    GCObject *o = obj2gco(uv);
    lua_assert(!isblack(o) && uv->v != &uv->u.value);
    L->openupval = uv->next;  /* remove from `open' list */
#if LUA_REFCOUNT
	if (--uv->ref == 0) {
      uv->marked ^= otherwhite(g);
      luarc_release(L, &uv->u.value);
	}
    if (uv->next)
      uv->next->gch.prev = NULL;
#endif /* LUA_REFCOUNT */
    if (isdead(g, o))
      luaF_freeupval(L, uv);  /* free upvalue */
    else {
      unlinkupval(uv);
      setobj(L, &uv->u.value, uv->v);
      uv->v = &uv->u.value;  /* now current value lives here */
      luaC_linkupval(L, uv);  /* link upvalue into `gcroot' list */
    }
  }
}


Proto *luaF_newproto (lua_State *L) {
#if LUA_MEMORY_STATS
  Proto *f;
  luaM_setname(L, "lua.proto");
  f = luaM_new(L, Proto);
#else
  Proto *f = luaM_new(L, Proto);
#endif /* LUA_MEMORY_STATS */
#if LUA_REFCOUNT
  f->ref = 0;
#endif /* LUA_REFCOUNT */
  luaC_link(L, obj2gco(f), LUA_TPROTO);
  f->k = NULL;
  f->sizek = 0;
  f->p = NULL;
  f->sizep = 0;
  f->code = NULL;
  f->sizecode = 0;
  f->sizelineinfo = 0;
  f->sizeupvalues = 0;
  f->nups = 0;
  f->upvalues = NULL;
  f->numparams = 0;
  f->is_vararg = 0;
  f->maxstacksize = 0;
  f->lineinfo = NULL;
  f->sizelocvars = 0;
  f->locvars = NULL;
  f->linedefined = 0;
  f->lastlinedefined = 0;
  f->source = NULL;
#if LUA_MEMORY_STATS
  luaM_setname(L, 0);
#endif /* LUA_MEMORY_STATS */
  return f;
}


void luaF_freeproto (lua_State *L, Proto *f) {
  luaM_freearray(L, f->code, f->sizecode, Instruction);
  luaM_freearray(L, f->p, f->sizep, Proto *);
  luaM_freearray(L, f->k, f->sizek, TValue);
  luaM_freearray(L, f->lineinfo, f->sizelineinfo, int);
  luaM_freearray(L, f->locvars, f->sizelocvars, struct LocVar);
  luaM_freearray(L, f->upvalues, f->sizeupvalues, TString *);
  luaM_free(L, f);
}


void luaF_freeclosure (lua_State *L, Closure *c) {
  int size = (c->c.isC) ? sizeCclosure(c->c.nupvalues) :
                          sizeLclosure(c->l.nupvalues);
  luaM_freemem(L, c, size);
}


/*
** Look for n-th local variable at line `line' in function `func'.
** Returns NULL if not found.
*/
const char *luaF_getlocalname (const Proto *f, int local_number, int pc) {
  int i;
  for (i = 0; i<f->sizelocvars && f->locvars[i].startpc <= pc; i++) {
    if (pc < f->locvars[i].endpc) {  /* is variable active? */
      local_number--;
      if (local_number == 0)
        return getstr(f->locvars[i].varname);
    }
  }
  return NULL;  /* not found */
}

NAMESPACE_LUA_END
