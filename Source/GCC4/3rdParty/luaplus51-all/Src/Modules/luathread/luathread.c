/*======================================================================*\
* LuaThreads: multi-(platform|threading) support for the Lua language.
* Diego Nehab, 12/3/2001
* RCS Id: $Id: luathread.c,v 1.13 2004/12/13 05:13:51 diego Exp $
\*======================================================================*/

#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>
#include "compat-5.1.h"

#include "pt.h"
#include "srm.h"
#include "auxiliar.h"

#include "luathread.h"

typedef struct t_states {
    lua_State *parent;
    lua_State *child;
} t_states;
typedef t_states *p_states;

static void *thread_entry(void *arg) {
    p_states st = (p_states) arg;
    int n = 1; 
    /* right now, we have the args table and the function on the stack */
    /* extract function arguments from args table */
    while (1) {
        lua_rawgeti(st->child, 1, n++);
        if (lua_isnil(st->child, -1)) break;
    }
    /* args table is garbage now */
    lua_remove(st->child, 1); 
    /* detach thread and invoke lua function with args */
    pthread_detach(pthread_self());
    lua_call(st->child, n-1, 0);
    /* kill registry reference to thread object */
    lua_pushlightuserdata(st->parent, st->child);
    lua_pushnil(st->parent);
    lua_settable(st->parent, LUA_REGISTRYINDEX);
    /* clean up */
	free(st);
    pthread_cleanup(pthread_self());
    return NULL;
}

static int newthread(lua_State *parent) {
    pthread_t thread;
    p_states st = (p_states) malloc(sizeof(t_states));
    if (!st) luaL_error(parent, "out of memory");
    st->parent = parent;
    luaL_checktype(st->parent, 1, LUA_TFUNCTION);
    luaL_checktype(st->parent, 2, LUA_TTABLE);
    lua_settop(st->parent, 2);
    st->child = lua_newthread(st->parent);
    if (st->child == NULL) luaL_error(st->parent, "cannot create new stack");
    /* create a hard reference to the thread object into the registry */
    lua_pushlightuserdata(st->parent, st->child);
    lua_insert(st->parent, -2);
    lua_settable(st->parent, LUA_REGISTRYINDEX);
    /* kill extra mutex created by lua_newthread and copy parent's over it */
    pthread_mutex_destroy(lt_mutex(st->child));
	lt_mutex(st->child) = lt_mutex(st->parent);
    /* move args table and function to child's stack */
    lua_xmove(st->parent, st->child, 1);
    lua_xmove(st->parent, st->child, 1);
    /* create a new thread of execution and we are done */
    if (pthread_create(&thread, NULL, thread_entry, st) != 0)
        luaL_error(st->parent, "cannot create new thread");
    return 0;
}

static int newmutex(lua_State *L) {
    srm_t *srm = (srm_t *) lua_newuserdata(L, sizeof(srm_t)); 
    if (srm_init(srm) != 0) luaL_error(L, "unable to create mutex");
    aux_setclass(L, "mutex", -1);
    return 1;
}

static int mutex_destroy(lua_State *L) {
    srm_destroy((srm_t *) lua_touserdata(L, 1));
    return 0;
}

static int mutex_lock(lua_State *L) {
    srm_t *srm = (srm_t *) aux_checkclass(L, "mutex", 1);
    if (srm_lock(srm) != 0) luaL_error(L, "unable to lock mutex");
    return 0;
}

static int mutex_unlock(lua_State *L) {
    srm_t *srm = (srm_t *) aux_checkclass(L, "mutex", 1);
    if (srm_unlock(srm) != 0) luaL_error(L, "unable to unlock mutex");
    return 0;
}

static int newcond(lua_State *L) {
    pthread_cond_t *cond = (pthread_cond_t *) 
        lua_newuserdata(L, pthread_cond_sizeof());
    aux_setclass(L, "cond", -1);
    if (pthread_cond_init(cond, NULL) != 0)
        luaL_error(L, "unable to create cond");
    return 1;
}

static int cond_wait(lua_State *L) {
    pthread_cond_t *cond = (pthread_cond_t *) aux_checkclass(L, "cond", 1);
    srm_t *srm = (srm_t *) aux_checkclass(L, "mutex", 2);
    if (srm_cond_wait(cond, srm) != 0)
        luaL_error(L, "unable to wait");
    return 0;
}

static int cond_signal(lua_State *L) {
    pthread_cond_t *cond = (pthread_cond_t *) aux_checkclass(L, "cond", 1);
    if (pthread_cond_signal(cond) != 0) luaL_error(L, "unable to signal");
    return 0;
}

static int cond_broadcast (lua_State *L) {
    pthread_cond_t *cond = (pthread_cond_t *) aux_checkclass(L, "cond", 1);
    if (pthread_cond_broadcast(cond) != 0) luaL_error(L, "unable to broadcast");
    return 0;
}

static int cond_destroy (lua_State *L) {
    pthread_cond_destroy((pthread_cond_t *) lua_touserdata(L, 1));
    return 0;
}

static const struct luaL_reg lib_ops[] = {
    {"newthread", newthread},
    {"newmutex", newmutex},
    {"newcond", newcond},
    {NULL, NULL}
};

static const struct luaL_reg mutex_ops[] = {
    {"lock", mutex_lock},
    {"unlock", mutex_unlock},
    {"__gc", mutex_destroy},
    {NULL, NULL}
};

static const struct luaL_reg cond_ops[] = {
    {"wait", cond_wait},
    {"signal", cond_signal},
    {"broadcast", cond_broadcast},
    {"__gc", cond_destroy},
    {NULL, NULL}
};

void luastateopen_thread(lua_State *L) {
    lt_mutex(L) = (pthread_mutex_t *) malloc(pthread_mutex_sizeof());
    pthread_mutex_init(lt_mutex(L), NULL);
}

int luaopen_thread(lua_State *L) {
    aux_open(L);
    aux_newclass(L, "mutex", mutex_ops);
    aux_newclass(L, "cond", cond_ops);
    luaL_module(L,  "thread", lib_ops, 0);
    lua_pushstring(L, "VERSION");
    lua_pushstring(L, LUATHREAD_VERSION);
    lua_settable(L, -3);
    return 1;
}
