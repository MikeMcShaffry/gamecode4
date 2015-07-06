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

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include <QVariant>
#include <QIcon>
#include <QLocale>
//#include <iostream>

#define LQT_POINTERS "Registry Pointers"
#define LQT_ENUMS "Registry Enumerations"

template<typename T> class LuaBinder;

extern int& lqtL_tointref (lua_State *, int);

extern void lqtL_pusharguments (lua_State *, char**);
extern char** lqtL_toarguments (lua_State *, int);
extern bool lqtL_testarguments (lua_State *, int);

extern void lqtL_manageudata (lua_State *, int);
extern void lqtL_unmanageudata (lua_State *, int);
extern void lqtL_pushudata (lua_State *, const void *, const char *);
extern void lqtL_passudata (lua_State *, const void *, const char *);
extern void * lqtL_toudata (lua_State *, int, const char *);
extern bool lqtL_testudata (lua_State *, int, const char *);
//#define lqtL_checkudata(a...) luaL_checkudata(a)
extern void * lqtL_checkudata (lua_State *, int, const char *);
#define lqtL_isudata(a...) lqtL_testudata(a)

extern void lqtL_pushenum (lua_State *, int, const char *);
extern bool lqtL_isenum (lua_State *, int, const char *);
extern int lqtL_toenum (lua_State *, int, const char *);


extern int lqtL_baseindex (lua_State *, int, int);

extern int lqtL_gc (lua_State *);
extern int lqtL_index (lua_State *);
extern int lqtL_newindex (lua_State *);


#endif // __LQT_COMMON_HPP

