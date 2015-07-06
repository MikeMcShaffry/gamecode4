/*======================================================================*\
* LuaThreads: multi-(platform|threading) support for the Lua language.
* Diego Nehab, 12/3/2001
* RCS Id: $Id: luathread.h,v 1.8 2004/11/26 07:59:36 diego Exp $
\*======================================================================*/
#ifndef LUATHREAD_H
#define LUATHREAD_H

#include "pt.h"

#define LUATHREAD_VERSION    "LuaThread 1.1 (alpha)"
#define LUATHREAD_COPYRIGHT  "Copyright (C) 2004-2005 Diego Nehab"
#define LUATHREAD_AUTHORS    "Diego Nehab"

/*-----------------------------------------------------------------------*\
* Macros do be included in EVERY Lua distribution file to be compiled.
\*-----------------------------------------------------------------------*/
typedef pthread_mutex_t   *pthread_mutex_p;
#define lt_back(L)        (((unsigned char *) L) - sizeof(pthread_mutex_p))
#define lt_mutex(L)       (*((pthread_mutex_p *) lt_back(L))) 
#define lua_lock(L)	      pthread_mutex_lock(lt_mutex(L))
#define lua_unlock(L)     pthread_mutex_unlock(lt_mutex(L))
#define LUA_USERSTATE     pthread_mutex_p
#define lua_userstateopen luastateopen_thread

/*-----------------------------------------------------------------------*\
* Initializes the LUA_USERSTATE data. To be called before ANY function
* that uses a given lua_State.
\*-----------------------------------------------------------------------*/
void luastateopen_thread(lua_State *L);

/*-----------------------------------------------------------------------*\
* Initializes the LuaThreads library, making available to Lua scripts the
* newthread, newmutex and newcond functions.
\*-----------------------------------------------------------------------*/
int luaopen_thread(lua_State *L);

#endif
