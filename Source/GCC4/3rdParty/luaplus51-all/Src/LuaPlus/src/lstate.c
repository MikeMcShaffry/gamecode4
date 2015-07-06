/*
** $Id: lstate.c,v 2.36.1.2 2008/01/03 15:20:39 roberto Exp $
** Global State
** See Copyright Notice in lua.h
*/


#include <stddef.h>

#define lstate_c
#define LUA_CORE

#include "lua.h"

#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lgc.h"
#include "llex.h"
#include "lmem.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "ltm.h"

NAMESPACE_LUA_BEGIN

#define state_size(x)	(sizeof(x) + LUAI_EXTRASPACE)
#define fromstate(l)	(cast(lu_byte *, (l)) - LUAI_EXTRASPACE)
#define tostate(l)   (cast(lua_State *, cast(lu_byte *, l) + LUAI_EXTRASPACE))


/*
** Main thread combines a thread state and the global state
*/
typedef struct LG {
  lua_State l;
  global_State g;
} LG;
  
#if LUAPLUS_EXTENSIONS

LUA_API void lua_setloadnotifyfunction(lua_State *L, void (*loadNotifyFunction)(lua_State *L, const char *))
{
  G(L)->loadNotifyFunction = loadNotifyFunction;
}


LUA_API void lua_setusergcfunction(lua_State *L, void (*userGCFunction)(void*))
{
  G(L)->userGCFunction = userGCFunction;
}


#endif /* LUAPLUS_EXTENSIONS */


static void stack_init (lua_State *L1, lua_State *L) {
#if LUA_MEMORY_STATS
  luaM_setname(L, "lua.callinfo");
#endif /* LUA_MEMORY_STATS */
  /* initialize CallInfo array */
  L1->base_ci = luaM_newvector(L, BASIC_CI_SIZE, CallInfo);
  L1->ci = L1->base_ci;
  L1->size_ci = BASIC_CI_SIZE;
  L1->end_ci = L1->base_ci + L1->size_ci - 1;
  /* initialize stack array */
#if LUA_MEMORY_STATS
  luaM_setname(L, "lua.stack");
#endif /* LUA_MEMORY_STATS */
  L1->stack = luaM_newvector(L, BASIC_STACK_SIZE + EXTRA_STACK, TValue);
  L1->stacksize = BASIC_STACK_SIZE + EXTRA_STACK;
  L1->top = L1->stack;
  L1->stack_last = L1->stack+(L1->stacksize - EXTRA_STACK)-1;
#if LUA_REFCOUNT
  luarc_newarray(L1->stack, L1->stack + L1->stacksize);
#endif /* LUA_REFCOUNT */
  /* initialize first ci */
  L1->ci->func = L1->top;
#if LUA_REFCOUNT  
  setnilvalue2n(L1, L1->top++);  /* `function' entry for this `ci' */
#else
  setnilvalue(L1->top++);  /* `function' entry for this `ci' */
#endif /* LUA_REFCOUNT */
  L1->base = L1->ci->base = L1->top;
  L1->ci->top = L1->top + LUA_MINSTACK;
#if LUA_EXT_RESUMABLEVM
  L1->ci->errfunc = 0;
#endif /* LUA_EXT_RESUMABLEVM */
#if LUA_MEMORY_STATS
  luaM_setname(L, 0);
#endif /* LUA_MEMORY_STATS */
}


static void freestack (lua_State *L, lua_State *L1) {
  luaM_freearray(L, L1->base_ci, L1->size_ci, CallInfo);
  luaM_freearray(L, L1->stack, L1->stacksize, TValue);
}


/*
** open parts that may cause memory-allocation errors
*/
#if LUA_EXT_RESUMABLEVM
static int f_luaopen (lua_State *L, void *ud) {
#else
static void f_luaopen (lua_State *L, void *ud) {
#endif /* LUA_EXT_RESUMABLEVM */
  global_State *g = G(L);
  UNUSED(ud);
  stack_init(L, L);  /* init stack */
#if LUA_MEMORY_STATS
  luaM_setname(L, "lua.globals");
#endif /* LUA_MEMORY_STATS */
  sethvalue(L, gt(L), luaH_new(L, 0, 2));  /* table of globals */
#if LUA_MEMORY_STATS
  luaM_setname(L, "lua.registry");
#endif /* LUA_MEMORY_STATS */
  sethvalue(L, registry(L), luaH_new(L, 0, 2));  /* registry */
#if LUA_FASTREF_SUPPORT
  {
    TValue n;

    sethvalue(L, &G(L)->l_refs, luaH_new(L, 0, 2));  /* refs */
	setnvalue(&n, 0);
    setobj2t(L, luaH_setnum(L, hvalue(&G(L)->l_refs), LUA_RIDX_FASTREF_FREELIST), &n);

    setnilvalue(&g->fastrefNilValue);
  }
#endif /* LUA_FASTREF_SUPPORT */
  luaS_resize(L, MINSTRTABSIZE);  /* initial size of string table */
  luaT_init(L);
  luaX_init(L);
  luaS_fix(luaS_newliteral(L, MEMERRMSG));
  g->GCthreshold = 4*g->totalbytes;
#if LUA_EXT_RESUMABLEVM
  return 0;
#endif /* LUA_EXT_RESUMABLEVM */
}


static void preinit_state (lua_State *L, global_State *g) {
  G(L) = g;
  L->stack = NULL;
  L->stacksize = 0;
  L->errorJmp = NULL;
  L->hook = NULL;
  L->hookmask = 0;
  L->basehookcount = 0;
#if !LUA_EXT_RESUMABLEVM
  L->allowhook = 1;
#endif /* LUA_EXT_RESUMABLEVM */
  resethookcount(L);
  L->openupval = NULL;
  L->size_ci = 0;
#if LUA_EXT_RESUMABLEVM
  L->nCcalls = LUA_NOYIELD | LUA_NOVPCALL;
#else
  L->nCcalls = L->baseCcalls = 0;
#endif /* LUA_EXT_RESUMABLEVM */
  L->status = 0;
  L->base_ci = L->ci = NULL;
#if LUA_EXT_RESUMABLEVM
  L->ctx = NULL;
#else
  L->savedpc = NULL;
  L->errfunc = 0;
#endif /* LUA_EXT_RESUMABLEVM */
#if LUA_REFCOUNT    
  L->ref = 0;
  setnilvalue2n(L, gt(L));
#else
  setnilvalue(gt(L));
#endif /* LUA_REFCOUNT */
}


static void close_state (lua_State *L) {
  global_State *g = G(L);
  luaF_close(L, L->stack);  /* close all upvalues for this thread */
  luaC_freeall(L);  /* collect all objects */
  lua_assert(g->rootgc == obj2gco(L));
  lua_assert(g->strt.nuse == 0);
  luaM_freearray(L, G(L)->strt.hash, G(L)->strt.size, TString *);
  luaZ_freebuffer(L, &g->buff);
  freestack(L, L);
//  lua_assert(g->totalbytes == sizeof(LG));
#if LUAPLUS_EXTENSIONS
  (*g->frealloc)(g->ud, fromstate(L), state_size(LG), 0, "lua.state", 0);
#else
  (*g->frealloc)(g->ud, fromstate(L), state_size(LG), 0);
#endif /* LUAPLUS_EXTENSIONS */
}


lua_State *luaE_newthread (lua_State *L) {
  lua_State *L1;
#if LUA_MEMORY_STATS
  luaM_setname(L, "lua_State");
#endif /* LUA_MEMORY_STATS */
  L1 = tostate(luaM_malloc(L, state_size(lua_State)));
  luaC_link(L, obj2gco(L1), LUA_TTHREAD);
  preinit_state(L1, G(L));
  stack_init(L1, L);  /* init stack */
  setobj2n(L, gt(L1), gt(L));  /* share table of globals */
  L1->hookmask = L->hookmask;
  L1->basehookcount = L->basehookcount;
  L1->hook = L->hook;
  resethookcount(L1);
  lua_assert(iswhite(obj2gco(L1)));
  return L1;
}

void luaE_freethread (lua_State *L, lua_State *L1) {
  luaF_close(L1, L1->stack);  /* close all upvalues for this thread */
  lua_assert(L1->openupval == NULL);
  luai_userstatefree(L1);
  freestack(L, L1);
  luaM_freemem(L, fromstate(L1), state_size(lua_State));
}

#if LUAPLUS_EXTENSIONS
void LuaState_UserStateOpen(lua_State* L);
#endif /* LUAPLUS_EXTENSIONS */

LUA_API lua_State *lua_newstate (lua_Alloc f, void *ud) {
  int i;
  lua_State *L;
  global_State *g;
#if LUAPLUS_EXTENSIONS
  void *l = (*f)(ud, NULL, 0, state_size(LG), "lua_State", 0);
#else
  void *l = (*f)(ud, NULL, 0, state_size(LG));
#endif /* LUAPLUS_EXTENSIONS */
  if (l == NULL) return NULL;
  L = tostate(l);
  g = &((LG *)L)->g;
  L->next = NULL;
#if LUA_REFCOUNT
  L->prev = NULL;
#endif /* LUA_REFCOUNT */
  L->tt = LUA_TTHREAD;
  g->currentwhite = bit2mask(WHITE0BIT, FIXEDBIT);
  L->marked = luaC_white(g);
  set2bits(L->marked, FIXEDBIT, SFIXEDBIT);
  preinit_state(L, g);
  g->frealloc = f;
  g->ud = ud;
  g->mainthread = L;
  g->uvhead.u.l.prev = &g->uvhead;
  g->uvhead.u.l.next = &g->uvhead;
  g->GCthreshold = 0;  /* mark it as unfinished state */
  g->strt.size = 0;
  g->strt.nuse = 0;
  g->strt.hash = NULL;
#if LUA_REFCOUNT    
  setnilvalue2n(L, registry(L));
#else
  setnilvalue(registry(L));
#endif /* LUA_REFCOUNT */
  luaZ_initbuffer(L, &g->buff);
  g->panic = NULL;
  g->gcstate = GCSpause;
  g->rootgc = obj2gco(L);
  g->sweepstrgc = 0;
  g->sweepgc = &g->rootgc;
  g->gray = NULL;
  g->grayagain = NULL;
  g->weak = NULL;
  g->tmudata = NULL;
  g->totalbytes = sizeof(LG);
  g->gcpause = LUAI_GCPAUSE;
  g->gcstepmul = LUAI_GCMUL;
  g->gcdept = 0;
#if LUAPLUS_EXTENSIONS
  g->loadNotifyFunction = NULL;
  g->userGCFunction = NULL;
  g->gchead_next = &g->gctail_next;
  g->gchead_prev = NULL;
  g->gctail_next = NULL;
  g->gctail_prev = &g->gchead_next;
#endif /* LUAPLUS_EXTENSIONS */
  for (i=0; i<NUM_TAGS; i++) g->mt[i] = NULL;
  if (luaD_rawrunprotected(L, f_luaopen, NULL) != 0) {
    /* memory allocation error: free partial state */
    close_state(L);
    L = NULL;
  }
  else
    luai_userstateopen(L);
  return L;
}


#if LUA_EXT_RESUMABLEVM
static int callallgcTM (lua_State *L, void *ud) {
  UNUSED(ud);
  luaC_callGCTM(L);  /* call GC metamethods for all udata */
  return 0;
}
#else
static void callallgcTM (lua_State *L, void *ud) {
  UNUSED(ud);
  luaC_callGCTM(L);  /* call GC metamethods for all udata */
}
#endif /* LUA_EXT_RESUMABLEVM */


LUA_API void lua_close (lua_State *L) {
  L = G(L)->mainthread;  /* only the main thread can be closed */
  lua_lock(L);
  luaF_close(L, L->stack);  /* close all upvalues for this thread */
  luaC_separateudata(L, 1);  /* separate udata that have GC metamethods */
#if !LUA_EXT_RESUMABLEVM
  L->errfunc = 0;  /* no error function during GC metamethods */
#endif /* LUA_EXT_RESUMABLEVM */
  do {  /* repeat until no more errors */
    L->ci = L->base_ci;
#if LUA_EXT_RESUMABLEVM
    L->ci->errfunc = 0;  /* no error function during GC metamethods */
#endif /* LUA_EXT_RESUMABLEVM */
    L->base = L->top = L->ci->base;
#if LUA_EXT_RESUMABLEVM
    L->nCcalls = 0;
#else
    L->nCcalls = L->baseCcalls = 0;
#endif /* LUA_EXT_RESUMABLEVM */
  } while (luaD_rawrunprotected(L, callallgcTM, NULL) != 0);
  lua_assert(G(L)->tmudata == NULL);
  luai_userstateclose(L);
  close_state(L);
}

NAMESPACE_LUA_END
