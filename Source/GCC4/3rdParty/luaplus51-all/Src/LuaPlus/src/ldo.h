/*
** $Id: ldo.h,v 2.7.1.1 2007/12/27 13:02:25 roberto Exp $
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/

#ifndef ldo_h
#define ldo_h


#include "lobject.h"
#include "lstate.h"
#include "lzio.h"

NAMESPACE_LUA_BEGIN

#define luaD_checkstack(L,n)	\
  if ((char *)L->stack_last - (char *)L->top <= (n)*(int)sizeof(TValue)) \
    luaD_growstack(L, n); \
  else condhardstacktests(luaD_reallocstack(L, L->stacksize - EXTRA_STACK - 1));


#define incr_top(L) {luaD_checkstack(L,1); L->top++;}

#define savestack(L,p)		((char *)(p) - (char *)L->stack)
#define restorestack(L,n)	((TValue *)((char *)L->stack + (n)))

#define saveci(L,p)		((char *)(p) - (char *)L->base_ci)
#define restoreci(L,n)		((CallInfo *)((char *)L->base_ci + (n)))


/* results from luaD_precall */
#define PCRLUA		0	/* initiated a call to a Lua function */
#define PCRC		1	/* did a call to a C function */
#define PCRYIELD	2	/* C function yielded */
 
#if LUA_EXT_RESUMABLEVM

/* call flags for luaD_call, stored in least significant bits of nCcalls */
#define LUA_NOHOOKS	1
#define LUA_NOVPCALL	2
#define LUA_NOYIELD	4

#define nohooks(L)	(L->nCcalls & LUA_NOHOOKS)
#define novpcall(L)	(L->nCcalls & LUA_NOVPCALL)
#define noyield(L)	(L->nCcalls & LUA_NOYIELD)

#define notresumable(L, stmt) \
	{ unsigned short save_ncc = L->nCcalls; \
	  L->nCcalls = save_ncc | (LUA_NOYIELD | LUA_NOVPCALL); \
	  stmt \
	  L->nCcalls = save_ncc; }

/* type of protected functions, to be run by `runprotected' */
typedef int (*Pfunc) (lua_State *L, void *ud);

#else

/* type of protected functions, to be ran by `runprotected' */
typedef void (*Pfunc) (lua_State *L, void *ud);

#endif /* LUA_EXT_RESUMABLEVM */

LUAI_FUNC int luaD_protectedparser (lua_State *L, ZIO *z, const char *name);
LUAI_FUNC void luaD_callhook (lua_State *L, int event, int line);
LUAI_FUNC int luaD_precall (lua_State *L, StkId func, int nresults);
#if LUA_EXT_RESUMABLEVM
LUAI_FUNC void luaD_call (lua_State *L, StkId func, int nresults, int callflags);
#else
LUAI_FUNC void luaD_call (lua_State *L, StkId func, int nResults);
LUAI_FUNC int luaD_pcall (lua_State *L, Pfunc func, void *u,
                                        ptrdiff_t oldtop, ptrdiff_t ef);
#endif /* LUA_EXT_RESUMABLEVM */
LUAI_FUNC int luaD_poscall (lua_State *L, StkId firstResult);
LUAI_FUNC void luaD_reallocCI (lua_State *L, int newsize);
LUAI_FUNC void luaD_reallocstack (lua_State *L, int newsize);
LUAI_FUNC void luaD_growstack (lua_State *L, int n);

LUAI_FUNC void luaD_throw (lua_State *L, int errcode);
LUAI_FUNC int luaD_rawrunprotected (lua_State *L, Pfunc f, void *ud);
#if LUA_EXT_RESUMABLEVM
LUAI_FUNC int luaD_pcall (lua_State *L, Pfunc func, void *ud,
                          ptrdiff_t old_top, int ef, unsigned int flagmask);

#define luaD_catch(L, ef) \
	{ lua_assert((ef) + 1 >= 0 && (ef) + 1 <= 255); \
	  L->ci->hookmask = L->hookmask; \
	  L->ci->errfunc = (ef) + 1; }
#endif /* LUA_EXT_RESUMABLEVM */

LUAI_FUNC void luaD_seterrorobj (lua_State *L, int errcode, StkId oldtop);

NAMESPACE_LUA_END

#endif

