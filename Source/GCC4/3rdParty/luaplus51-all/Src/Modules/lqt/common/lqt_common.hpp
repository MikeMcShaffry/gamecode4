/*
 * Copyright (c) 2007-2008 Mauro Iazzi
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef __LQT_COMMON_HPP
#define __LQT_COMMON_HPP

#ifdef _WIN32
#define LQT_EXPORT  __declspec(dllexport)
#else
#define LQT_EXPORT
#endif

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

//#include <iostream>

//#define lqtL_register(L, p, n) ( (void)L, (void)p, (void)n )
//#define lqtL_unregister(L, p) ( (void)L, (void)p )

#define LQT_POINTERS "Registry Pointers"
#define LQT_REFS "Registry References"
#define LQT_ENUMS "Registry Enums"
#define LQT_PCALL "Registry PCall Pointer"
#define LQT_SUPER "SUPER"

// Qt-specific fields
#define LQT_METACALLER "Registry MetaCaller"
#define LQT_OBJMETASTRING "Lqt MetaStringData"
#define LQT_OBJMETADATA "Lqt MetaData"
#define LQT_OBJSLOTS "Lqt Slots"
#define LQT_OBJSIGS "Lqt Signatures"

// macro to ge positive indexes
#define LQT_TOPOSITIVE(L, i) (((i)<0)?(lua_gettop(L)+1+(i)):(i))

// use standard pcall by default
#ifndef LQT_DEBUG
#define lqtL_pcall(L, n, r, e) lua_pcall(L, n, r, e)
#else  // LQT_DEBUG
#define lqtL_pcall(L, n, r, e) lqtL_pcall_debug(L, n, r, e)
#endif // LQT_DEBUG

typedef int (*lqt_PCallPtr) (lua_State *L, int nargs, int nresults, int errfunc);

void lqtL_register(lua_State *, const void *);
void lqtL_unregister(lua_State *, const void *);

//int& lqtL_tointref (lua_State *, int);

//void lqtL_pusharguments (lua_State *, char**);
//char** lqtL_toarguments (lua_State *, int);
//bool lqtL_testarguments (lua_State *, int);

//void lqtL_manageudata (lua_State *, int);
//void lqtL_unmanageudata (lua_State *, int);
void lqtL_pushudata (lua_State *, const void *, const char *);
void lqtL_passudata (lua_State *, const void *, const char *);
void lqtL_copyudata (lua_State *, const void *, const char *);
void * lqtL_toudata (lua_State *, int, const char *);
bool lqtL_testudata (lua_State *, int, const char *);
//#define lqtL_checkudata(a...) luaL_checkudata(a)
void * lqtL_checkudata (lua_State *, int, const char *);
void lqtL_eraseudata (lua_State *, int, const char *);
#define lqtL_isudata lqtL_testudata


bool lqtL_canconvert(lua_State *L, int n, const char *to_type);
void *lqtL_convert(lua_State *L, int n, const char *to_type);
typedef bool  (*lqt_testfunc) (lua_State *L, int n);
typedef void* (*lqt_convertfunc) (lua_State *L, int n);


void lqtL_pushenum (lua_State *, int, const char *);
bool lqtL_isenum (lua_State *, int, const char *);
int lqtL_toenum (lua_State *, int, const char *);

bool lqtL_isinteger (lua_State *, int);
bool lqtL_isnumber (lua_State *, int);
bool lqtL_isstring (lua_State *, int);
bool lqtL_isboolean (lua_State *, int);

bool lqtL_missarg (lua_State *, int, int);
//int lqtL_baseindex (lua_State *, int, int);

//int lqtL_gc (lua_State *);
//int lqtL_index (lua_State *);
//int lqtL_newindex (lua_State *);

typedef struct {
    const char *name;
    int value;
} lqt_Enum;

typedef struct {
    lqt_Enum *enums;
    const char *name;
} lqt_Enumlist;

int lqtL_createenum (lua_State *L, lqt_Enum e[], const char *n);
int lqtL_createenumlist (lua_State *, lqt_Enumlist[]);

typedef struct {
    const char *basename;
    ptrdiff_t offset;
} lqt_Base;

typedef struct {
    luaL_Reg *mt;
    lqt_Base *bases;
    const char * name;
} lqt_Class;

int lqtL_createclass (lua_State *, const char *, luaL_Reg *, luaL_Reg *, luaL_Reg *, lqt_Base *);

/* functions to get/push special types */

bool * lqtL_toboolref (lua_State *, int);
void * lqtL_getref (lua_State *, size_t, bool);
int * lqtL_tointref (lua_State *, int);
char ** lqtL_toarguments (lua_State *, int);
void lqtL_pusharguments (lua_State *, char **);

int lqtL_getflags (lua_State *, int, const char *);
void lqtL_pushflags (lua_State *, int, const char *);
#define lqtL_isflags(L, i) (lua_istable((L), (i)) || lqtL_isinteger((L), (i)))

int lqtL_touintarray (lua_State *);

int lqtL_pcall_debug (lua_State *L, int narg, int nres, int err);

int lqtL_getoverload (lua_State *L, int index, const char *name);

const char * lqtL_source(lua_State *L, int idx);

bool lqtL_is_super(lua_State *L, int idx);
void lqtL_register_super(lua_State *L);

const char * lqtL_pushtrace(lua_State *L);
void lqtL_pushudatatype(lua_State *L, int index);
const char * lqtL_getarglist(lua_State *L);

void lqtL_selfcheck(lua_State *L, void *self, const char *name);

#endif // __LQT_COMMON_HPP

