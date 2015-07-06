/*
** $Id: lua.h,v 1.218.1.5 2008/08/06 13:30:12 roberto Exp $
** Lua - An Extensible Extension Language
** Lua.org, PUC-Rio, Brazil (http://www.lua.org)
** See Copyright Notice at the end of this file
*/


#ifndef lua_h
#define lua_h

#include <stdarg.h>
#include <stddef.h>

#include "../LuaLink.h"

#include "luaconf.h"

NAMESPACE_LUA_BEGIN

LUA_EXTERN_C_BEGIN

#define LUA_VERSION	"Lua 5.1"
#define LUA_RELEASE	"Lua 5.1.4"
#define LUA_VERSION_NUM	501
#define LUA_COPYRIGHT	"Copyright (C) 1994-2008 Lua.org, PUC-Rio"
#define LUA_AUTHORS 	"R. Ierusalimschy, L. H. de Figueiredo & W. Celes"


/* mark for precompiled code (`<esc>Lua') */
#define	LUA_SIGNATURE	"\033Lua"

/* option for multiple returns in `lua_pcall' and `lua_call' */
#define LUA_MULTRET	(-1)


/*
** pseudo-indices
*/
#define LUA_REGISTRYINDEX	(-10000)
#define LUA_ENVIRONINDEX	(-10001)
#define LUA_GLOBALSINDEX	(-10002)
#define lua_upvalueindex(i)	(LUA_GLOBALSINDEX-(i))


/* thread status; 0 is OK */
#define LUA_YIELD	1
#define LUA_ERRRUN	2
#define LUA_ERRSYNTAX	3
#define LUA_ERRMEM	4
#define LUA_ERRERR	5
#if LUA_EXT_RESUMABLEVM
#define LUA_ERREXC	6
#define LUA_ERRFORCECO  7
#endif /* LUA_EXT_RESUMABLEVM */

typedef struct lua_State lua_State;

typedef int (*lua_CFunction) (lua_State *L);

#if LUA_WIDESTRING
/* type of lex characters in Lua */
typedef unsigned short lua_WChar;
#endif /* LUA_WIDESTRING */

/*
** functions that read/write blocks when loading/dumping Lua chunks
*/
typedef const char * (*lua_Reader) (lua_State *L, void *ud, size_t *sz);

typedef int (*lua_Writer) (lua_State *L, const void* p, size_t sz, void* ud);


/*
** prototype for memory-allocation functions
*/
#if LUAPLUS_EXTENSIONS
typedef void * (*lua_Alloc) (void *ud, void *ptr, size_t osize, size_t nsize, const char* name, unsigned int alloc_flags);
#else
typedef void * (*lua_Alloc) (void *ud, void *ptr, size_t osize, size_t nsize);
#endif /* LUAPLUS_EXTENSIONS */


/*
** basic types
*/
#define LUA_TNONE		(-1)

#define LUA_TNIL		0
#define LUA_TBOOLEAN		1
#define LUA_TLIGHTUSERDATA	2
#define LUA_TNUMBER		3
#define LUA_TSTRING		4
#define LUA_TTABLE		5
#define LUA_TFUNCTION		6
#define LUA_TUSERDATA		7
#define LUA_TTHREAD		8
#if LUA_WIDESTRING
#define LUA_TWSTRING            9
#endif /* LUA_WIDESTRING */



/* minimum Lua stack available to a C function */
#define LUA_MINSTACK	20


/*
** generic extra include file
*/
#if defined(LUA_USER_H)
#include LUA_USER_H
#endif

#if LUA_FASTREF_SUPPORT
#define LUA_RIDX_FASTREF_FREELIST	1
#endif /* LUA_FASTREF_SUPPORT */


/* type of numbers in Lua */
typedef LUA_NUMBER lua_Number;


/* type for integer functions */
typedef LUA_INTEGER lua_Integer;



/*
** state manipulation
*/
LUA_API lua_State *(lua_newstate) (lua_Alloc f, void *ud);
LUA_API void       (lua_close) (lua_State *L);
LUA_API lua_State *(lua_newthread) (lua_State *L);

LUA_API lua_CFunction (lua_atpanic) (lua_State *L, lua_CFunction panicf);


/*
** basic stack manipulation
*/
LUA_API int   (lua_gettop) (lua_State *L);
LUA_API void  (lua_settop) (lua_State *L, int idx);
LUA_API void  (lua_pushvalue) (lua_State *L, int idx);
LUA_API void  (lua_remove) (lua_State *L, int idx);
LUA_API void  (lua_insert) (lua_State *L, int idx);
LUA_API void  (lua_replace) (lua_State *L, int idx);
LUA_API int   (lua_checkstack) (lua_State *L, int sz);

LUA_API void  (lua_xmove) (lua_State *from, lua_State *to, int n);


/*
** access functions (stack -> C)
*/

LUA_API int             (lua_isnumber) (lua_State *L, int idx);
LUA_API int             (lua_isstring) (lua_State *L, int idx);
LUA_API int             (lua_iscfunction) (lua_State *L, int idx);
LUA_API int             (lua_isuserdata) (lua_State *L, int idx);
LUA_API int             (lua_type) (lua_State *L, int idx);
LUA_API const char     *(lua_typename) (lua_State *L, int tp);

LUA_API int            (lua_equal) (lua_State *L, int idx1, int idx2);
LUA_API int            (lua_rawequal) (lua_State *L, int idx1, int idx2);
LUA_API int            (lua_lessthan) (lua_State *L, int idx1, int idx2);

LUA_API lua_Number      (lua_tonumber) (lua_State *L, int idx);
LUA_API lua_Integer     (lua_tointeger) (lua_State *L, int idx);
LUA_API int             (lua_toboolean) (lua_State *L, int idx);
LUA_API const char     *(lua_tolstring) (lua_State *L, int idx, size_t *len);
LUA_API size_t          (lua_objlen) (lua_State *L, int idx);
LUA_API lua_CFunction   (lua_tocfunction) (lua_State *L, int idx);
LUA_API void	       *(lua_touserdata) (lua_State *L, int idx);
LUA_API lua_State      *(lua_tothread) (lua_State *L, int idx);
LUA_API const void     *(lua_topointer) (lua_State *L, int idx);


/*
** push functions (C -> stack)
*/
LUA_API void  (lua_pushnil) (lua_State *L);
LUA_API void  (lua_pushnumber) (lua_State *L, lua_Number n);
LUA_API void  (lua_pushinteger) (lua_State *L, lua_Integer n);
LUA_API void  (lua_pushlstring) (lua_State *L, const char *s, size_t l);
LUA_API void  (lua_pushstring) (lua_State *L, const char *s);
LUA_API const char *(lua_pushvfstring) (lua_State *L, const char *fmt,
                                                      va_list argp);
LUA_API const char *(lua_pushfstring) (lua_State *L, const char *fmt, ...);
LUA_API void  (lua_pushcclosure) (lua_State *L, lua_CFunction fn, int n);
LUA_API void  (lua_pushboolean) (lua_State *L, int b);
LUA_API void  (lua_pushlightuserdata) (lua_State *L, void *p);
LUA_API int   (lua_pushthread) (lua_State *L);


/*
** get functions (Lua -> stack)
*/
LUA_API void  (lua_gettable) (lua_State *L, int idx);
LUA_API void  (lua_getfield) (lua_State *L, int idx, const char *k);
LUA_API void  (lua_rawget) (lua_State *L, int idx);
LUA_API void  (lua_rawgeti) (lua_State *L, int idx, int n);
LUA_API void  (lua_createtable) (lua_State *L, int narr, int nrec);
LUA_API void *(lua_newuserdata) (lua_State *L, size_t sz);
LUA_API int   (lua_getmetatable) (lua_State *L, int objindex);
LUA_API void  (lua_getfenv) (lua_State *L, int idx);


/*
** set functions (stack -> Lua)
*/
LUA_API void  (lua_settable) (lua_State *L, int idx);
LUA_API void  (lua_setfield) (lua_State *L, int idx, const char *k);
LUA_API void  (lua_rawset) (lua_State *L, int idx);
LUA_API void  (lua_rawseti) (lua_State *L, int idx, int n);
LUA_API int   (lua_setmetatable) (lua_State *L, int objindex);
LUA_API int   (lua_setfenv) (lua_State *L, int idx);


/*
** `load' and `call' functions (load and run Lua code)
*/
#if LUA_EXT_RESUMABLEVM
LUA_API void  (lua_vcall) (lua_State *L, int nargs, int nresults, void *ctx);
LUA_API int   (lua_vpcall) (lua_State *L, int nargs, int nresults,
                            int errfunc, void *ctx);
#else
LUA_API void  (lua_call) (lua_State *L, int nargs, int nresults);
LUA_API int   (lua_pcall) (lua_State *L, int nargs, int nresults, int errfunc);
#endif /* LUA_EXT_RESUMABLEVM */
LUA_API int   (lua_cpcall) (lua_State *L, lua_CFunction func, void *ud);
LUA_API int   (lua_load) (lua_State *L, lua_Reader reader, void *dt,
                                        const char *chunkname);
#if LUA_WIDESTRING
LUA_API int   (lua_wload) (lua_State *L, lua_Reader reader, void *data,
                      const char *chunkname);
#endif /* LUA_WIDESTRING */

LUA_API int (lua_dump) (lua_State *L, lua_Writer writer, void *data);

#if LUA_EXT_RESUMABLEVM
LUA_API void *lua_vcontext (lua_State *L);

#define lua_icontext(L)		((int)(ptrdiff_t)lua_vcontext(L))
#define lua_call(L, na, nr)	lua_vcall(L, (na), (nr), NULL)
#define lua_icall(L, na, nr, i) \
	lua_vcall(L, (na), (nr), (void *)(ptrdiff_t)(i))
#define lua_pcall(L, na, nr, ef)	lua_vpcall(L, (na), (nr), (ef), NULL)
#define lua_ipcall(L, na, nr, ef, i) \
	lua_vpcall(L, (na), (nr), (ef), (void *)(ptrdiff_t)(i))
#endif /* LUA_EXT_RESUMABLEVM */


/*
** coroutine functions
*/
#if LUA_EXT_RESUMABLEVM
LUA_API int  (lua_vyield) (lua_State *L, int nresults, void *ctx);
#else
LUA_API int  (lua_yield) (lua_State *L, int nresults);
#endif /* LUA_EXT_RESUMABLEVM */
LUA_API int  (lua_resume) (lua_State *L, int narg);
LUA_API int  (lua_status) (lua_State *L);

#if LUA_EXT_RESUMABLEVM
#define lua_yield(L, nr)	lua_vyield(L, (nr), NULL)
#define lua_iyield(L, nr, i)	lua_vyield(L, (nr), (void *)(ptrdiff_t)(i))
#endif /* LUA_EXT_RESUMABLEVM */


/*
** garbage-collection function and options
*/

#define LUA_GCSTOP		0
#define LUA_GCRESTART		1
#define LUA_GCCOLLECT		2
#define LUA_GCCOUNT		3
#define LUA_GCCOUNTB		4
#define LUA_GCSTEP		5
#define LUA_GCSETPAUSE		6
#define LUA_GCSETSTEPMUL	7

LUA_API int (lua_gc) (lua_State *L, int what, int data);


/*
** miscellaneous functions
*/

LUA_API int   (lua_error) (lua_State *L);

LUA_API int   (lua_next) (lua_State *L, int idx);

LUA_API void  (lua_concat) (lua_State *L, int n);

LUA_API lua_Alloc (lua_getallocf) (lua_State *L, void **ud);
LUA_API void lua_setallocf (lua_State *L, lua_Alloc f, void *ud);



/* 
** ===============================================================
** some useful macros
** ===============================================================
*/

#define lua_pop(L,n)		lua_settop(L, -(n)-1)

#define lua_newtable(L)		lua_createtable(L, 0, 0)

#define lua_register(L,n,f) (lua_pushcfunction(L, (f)), lua_setglobal(L, (n)))

#define lua_pushcfunction(L,f)	lua_pushcclosure(L, (f), 0)

#define lua_strlen(L,i)		lua_objlen(L, (i))

#define lua_isfunction(L,n)	(lua_type(L, (n)) == LUA_TFUNCTION)
#define lua_istable(L,n)	(lua_type(L, (n)) == LUA_TTABLE)
#define lua_islightuserdata(L,n)	(lua_type(L, (n)) == LUA_TLIGHTUSERDATA)
#define lua_isnil(L,n)		(lua_type(L, (n)) == LUA_TNIL)
#define lua_isboolean(L,n)	(lua_type(L, (n)) == LUA_TBOOLEAN)
#define lua_isthread(L,n)	(lua_type(L, (n)) == LUA_TTHREAD)
#define lua_isnone(L,n)		(lua_type(L, (n)) == LUA_TNONE)
#define lua_isnoneornil(L, n)	(lua_type(L, (n)) <= 0)

#define lua_pushliteral(L, s)	\
	lua_pushlstring(L, "" s, (sizeof(s)/sizeof(char))-1)

#define lua_setglobal(L,s)	lua_setfield(L, LUA_GLOBALSINDEX, (s))
#define lua_getglobal(L,s)	lua_getfield(L, LUA_GLOBALSINDEX, (s))

#define lua_tostring(L,i)	lua_tolstring(L, (i), NULL)



/*
** compatibility macros and functions
*/

#define lua_open()	luaL_newstate()

#define lua_getregistry(L)	lua_pushvalue(L, LUA_REGISTRYINDEX)

#define lua_getgccount(L)	lua_gc(L, LUA_GCCOUNT, 0)

#define lua_Chunkreader		lua_Reader
#define lua_Chunkwriter		lua_Writer


/* hack */
LUA_API void lua_setlevel	(lua_State *from, lua_State *to);


/*
** {======================================================================
** Debug API
** =======================================================================
*/


/*
** Event codes
*/
#define LUA_HOOKCALL	0
#define LUA_HOOKRET	1
#define LUA_HOOKLINE	2
#define LUA_HOOKCOUNT	3
#define LUA_HOOKTAILRET 4
#if LUA_TILDE_DEBUGGER
#define LUA_HOOKERROR	5
#endif /* LUA_TILDE_DEBUGGER */


/*
** Event masks
*/
#define LUA_MASKCALL	(1 << LUA_HOOKCALL)
#define LUA_MASKRET	(1 << LUA_HOOKRET)
#define LUA_MASKLINE	(1 << LUA_HOOKLINE)
#define LUA_MASKCOUNT	(1 << LUA_HOOKCOUNT)
#if LUA_TILDE_DEBUGGER
#define LUA_MASKERROR	(1 << LUA_HOOKERROR)
#endif /* LUA_TILDE_DEBUGGER */

typedef struct lua_Debug lua_Debug;  /* activation record */


/* Functions to be called by the debuger in specific events */
typedef void (*lua_Hook) (lua_State *L, lua_Debug *ar);


LUA_API int lua_getstack (lua_State *L, int level, lua_Debug *ar);
LUA_API int lua_getinfo (lua_State *L, const char *what, lua_Debug *ar);
LUA_API const char *lua_getlocal (lua_State *L, const lua_Debug *ar, int n);
LUA_API const char *lua_setlocal (lua_State *L, const lua_Debug *ar, int n);
LUA_API const char *lua_getupvalue (lua_State *L, int funcindex, int n);
LUA_API const char *lua_setupvalue (lua_State *L, int funcindex, int n);
#if LUA_TILDE_DEBUGGER
LUA_API int lua_getvararg (lua_State *L, const lua_Debug *ar, int n);
#endif /* LUA_TILDE_DEBUGGER */

LUA_API int lua_sethook (lua_State *L, lua_Hook func, int mask, int count);
LUA_API lua_Hook lua_gethook (lua_State *L);
LUA_API int lua_gethookmask (lua_State *L);
LUA_API int lua_gethookcount (lua_State *L);


struct lua_Debug {
  int event;
  const char *name;	/* (n) */
  const char *namewhat;	/* (n) `global', `local', `field', `method' */
  const char *what;	/* (S) `Lua', `C', `main', `tail' */
  const char *source;	/* (S) */
  int currentline;	/* (l) */
  int nups;		/* (u) number of upvalues */
  int linedefined;	/* (S) */
  int lastlinedefined;	/* (S) */
  char short_src[LUA_IDSIZE]; /* (S) */
  /* private part */
  int i_ci;  /* active function */
};

#if LUA_FASTREF_SUPPORT

#define LUA_FASTREFNIL	(-1999999)

LUA_API int lua_fastref (lua_State *L);
LUA_API int lua_fastrefindex (lua_State *L, int idx);
LUA_API void lua_fastunref (lua_State *L, int ref);
LUA_API void lua_getfastref (lua_State *L, int ref);

#endif /* LUA_FASTREF_SUPPORT */

LUA_EXTERN_C_END

/* }====================================================================== */

/******************************************************************************
* Copyright (C) 1994-2008 Lua.org, PUC-Rio.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

#if LUA_WIDESTRING

LUA_EXTERN_C_BEGIN

/* formats for Lua numbers */
#ifndef LUA_NUMBER_WSCAN
#define LUA_NUMBER_WSCAN	L"%lf"
#endif

#ifndef LUA_NUMBER_WFMT
#define LUA_NUMBER_WFMT_LOCAL const lua_WChar LUA_NUMBER_WFMT[] = { '%', '.', '1', '6', 'g', 0 }
#endif

LUA_API int             lua_iswstring (lua_State *L, int index);
LUA_API const lua_WChar  *lua_towstring (lua_State *L, int index);
LUA_API void  lua_pushlwstring (lua_State *L, const lua_WChar *s, size_t len);
LUA_API void  lua_pushwstring (lua_State *L, const lua_WChar *s);

LUA_API size_t lp_wcslen(const lua_WChar* str);
LUA_API int lp_wcscmp(const lua_WChar* str1, const lua_WChar* str2);

#define lua_pushwliteral(L, s)	\
	lua_pushlwstring(L, (const lua_WChar*)(L"" s), (sizeof(s)/sizeof(lua_WChar))-1)

#define LUA_WSTRLIBNAME	"wstring"
LUALIB_API int luaopen_wstring (lua_State *L);

LUALIB_API int luaL_loadwbuffer (lua_State *L, const lua_WChar *buff, size_t size, const char *name);

LUA_API const lua_WChar     *(lua_tolwstring) (lua_State *L, int idx, size_t *len);

size_t lua_WChar_len(const lua_WChar* str);

LUA_EXTERN_C_END

#endif /* LUA_WIDESTRING */

#if LUAPLUS_EXTENSIONS

LUA_EXTERN_C_BEGIN

#define LUA_ALLOC_TEMP 1
LUA_API void lua_getdefaultallocfunction(lua_Alloc* allocFunc, void** ud);
LUA_API void lua_setdefaultallocfunction(lua_Alloc reallocFunc, void* ud);

LUA_API void lua_setloadnotifyfunction(lua_State *L, void (*loadNotifyFunction)(lua_State *L, const char *));
LUA_API void lua_setusergcfunction(lua_State *L, void (*userGCFunction)(void*));

# define lua_boxpointer(L,u) \
        (*(void **)(lua_newuserdata(L, sizeof(void *))) = (u))

#define lua_unboxpointer(L,i)   (*(void **)(lua_touserdata(L, i)))

LUA_EXTERN_C_END

#endif /* LUAPLUS_EXTENSIONS */

LUA_EXTERN_C_BEGIN

#if LUA_ENDIAN_SUPPORT
LUA_API int (lua_dumpendian) (lua_State *L, lua_Writer writer, void *data, int strip, char endian);
#endif /* LUA_ENDIAN_SUPPORT */

LUA_EXTERN_C_END

NAMESPACE_LUA_END

#endif
