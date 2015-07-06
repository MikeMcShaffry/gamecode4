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

#include "lqt_common.hpp"
#include <iostream>
#include <cstdlib>
#include <cstring>

static void lqtL_getenumtable (lua_State *L) {
    lua_getfield(L, LUA_REGISTRYINDEX, LQT_ENUMS);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setfield(L, LUA_REGISTRYINDEX, LQT_ENUMS);
    }
}

static void lqtL_getpointertable (lua_State *L) {
    lua_getfield(L, LUA_REGISTRYINDEX, LQT_POINTERS); // (1) get storage for pointers
    if (lua_isnil(L, -1)) { // (1) if there is not
        lua_pop(L, 1); // (0) pop the nil value
        lua_newtable(L); // (1) create a new one
        lua_newtable(L); // (2) create an empty metatable
        lua_pushstring(L, "v"); // (3) push the mode value: weak values are enough
        lua_setfield(L, -2, "__mode"); // (2) set the __mode field
        lua_setmetatable(L, -2); // (1) set it as the metatable
        lua_pushvalue(L, -1); // (2) duplicate the new pointer table
        lua_setfield(L, LUA_REGISTRYINDEX, LQT_POINTERS); // (1) put one copy as storage
    }
}

static void lqtL_getreftable (lua_State *L) {
    lua_getfield(L, LUA_REGISTRYINDEX, LQT_REFS); // (1) get storage for pointers
    if (lua_isnil(L, -1)) { // (1) if there is not
        lua_pop(L, 1); // (0) pop the nil value
        lua_newtable(L); // (1) create a new one
        lua_newtable(L); // (2) create an empty metatable
        lua_pushstring(L, "v"); // (3) push the mode value: weak values are enough
        lua_setfield(L, -2, "__mode"); // (2) set the __mode field
        lua_setmetatable(L, -2); // (1) set it as the metatable
        lua_pushvalue(L, -1); // (2) duplicate the new pointer table
        lua_setfield(L, LUA_REGISTRYINDEX, LQT_REFS); // (1) put one copy as storage
    }
}

void * lqtL_getref (lua_State *L, size_t sz, bool weak) {
    void *ret = NULL;
    lqtL_getreftable(L); // (1)
    ret = lua_newuserdata(L, sz); // (2)
    /*
       lua_newtable(L); // (3)
       lua_getglobal(L, "DEBUG"); // (4)
       lua_setfield(L, -2, "__gc"); // (3)
       lua_setmetatable(L, -2); // (2)
       */
    if (weak) {
        lua_rawseti(L, -2, 1+lua_objlen(L, -2)); // (1)
    } else {
        lua_pushinteger(L, 1+lua_objlen(L, -2)); // (3)
        lua_settable(L, -3); // (1)
    }
    lua_pop(L, 1);
    return ret;
}

bool * lqtL_toboolref (lua_State *L, int index) {
    bool tmp = lua_toboolean(L, index) == 1;
    bool *ret = (bool*)lqtL_getref(L, sizeof(bool), true);
    *ret = tmp;
    return ret;
}

int * lqtL_tointref (lua_State *L, int index) {
    int tmp = lua_tointeger(L, index);
    int *ret = (int*)lqtL_getref(L, sizeof(int), false);
    *ret = tmp;
    return ret;
}

void lqtL_pusharguments (lua_State *L, char **argv) {
    int i = 0;
    lua_newtable(L);
    for (i=0;*argv /* fix the maximum number? */;argv++,i++) {
        lua_pushstring(L, *argv);
        lua_rawseti(L, -2, i+1);
    }
    return;
}

char ** lqtL_toarguments (lua_State *L, int index) {
    char ** ret = (char**)lqtL_getref(L, sizeof(char*)*(lua_objlen(L, index)+1), false);
    const char *str = NULL;
    size_t strlen = 0;
    int i = 0;
    for (i=0;;i++) {
        lua_rawgeti(L, index, i+1);
        if (!lua_isstring(L, -1)) {
            str = NULL; strlen = 0;
            ret[i] = NULL;
            lua_pop(L, 1);
            break;
        } else {
            str = lua_tolstring(L, -1, &strlen);
            ret[i] = (char*)lqtL_getref(L, sizeof(char)*(strlen+1), false);
            strncpy(ret[i], str, strlen+1);
            lua_pop(L, 1);
        }
    }
    return ret;
}

int lqtL_createenum (lua_State *L, lqt_Enum e[], const char *n) {
    luaL_Reg empty[] = { { 0, 0 } };
    lqt_Enum *l = e;
    lqtL_getenumtable(L); // (1)
    lua_newtable(L); // (2)
    lua_pushvalue(L, -1); // (3)
    lua_setfield(L, -3, n); // (2)
    while ( (l->name!=0) ) { // (2)
        lua_pushstring(L, l->name); // (3)
        lua_pushinteger(L, l->value); // (4)
        lua_settable(L, -3); // (2)
        lua_pushinteger(L, l->value); // (3)
        lua_pushstring(L, l->name); // (4)
        lua_settable(L, -3); // (2)
        l++; // (2)
    }
    lua_pop(L, 2); // (0)
    l = e;
    luaL_register(L, n, empty); // (1)
    while ( (l->name!=0) ) { // (1)
        lua_pushstring(L, l->name); // (2)
        lua_pushinteger(L, l->value); // (3)
        lua_settable(L, -3); // (1)
        lua_pushinteger(L, l->value); // (2)
        lua_pushstring(L, l->name); // (3)
        lua_settable(L, -3); // (1)
        l++; // (1)
    }
    lua_pop(L, 1); // (0)
    return 0;
}

int lqtL_createenumlist (lua_State *L, lqt_Enumlist list[]) {
    while (list->enums!=0 && list->name!=0) {
        lqtL_createenum(L, list->enums, list->name); // (0)
        list++;
    }
    return 0;
}

static int lqtL_gcfunc (lua_State *L) {
    if (!lua_isuserdata(L, 1) || lua_islightuserdata(L, 1)) return 0;
    lua_getfenv(L, 1); // (1)
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1); // (0)
        return 0;
    }
    lua_getfield(L, -1, "__gc"); // (2)
    lua_remove(L, -2); // (1)
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1); // (0)
        return 0;
    }
    lua_pushvalue(L, 1); // (2)
    if (lqtL_pcall(L, 1, 0, 0)) { // (-2;+1/+0)
        // (1)
        return lua_error(L);
    }
    return 0; // (+0)
}

static int lqtL_newindexfunc (lua_State *L) {
    if (!lua_isuserdata(L, 1) && lua_islightuserdata(L, 1)) return 0;
    lua_getmetatable(L, 1);
    lua_pushliteral(L, "__set");
    lua_rawget(L, -2);
    if (lua_istable(L, -1)) {
        lua_pushvalue(L, 2);
        lua_gettable(L, -2);
        if (lua_isfunction(L, -1)) {
            lua_CFunction setter = lua_tocfunction(L, -1);
            if (!setter) return luaL_error(L, "Invalid setter %s", lua_tostring(L, 2));
            return setter(L);
        }
    }
    lua_settop(L, 3); // (=3)
    lua_getfenv(L, 1); // (+1)
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1); // (+0)
        return 0;
    }
    lua_remove(L, 1); // (+0)
    lua_insert(L, 1); // (+0)
    lua_rawset(L, 1); // (-2)
    return 0;
}

int lqtL_getoverload (lua_State *L, int index, const char *name) {
    luaL_checkstack(L, 2, "no space to grow");
    if (lua_isuserdata(L, index) && !lua_islightuserdata(L, index)) {
        lua_getfenv(L, index); // (1)
        lua_getfield(L, -1, name); // (2)
        lua_remove(L, -2); // (1)
    } else {
        lua_pushnil(L); // (1)
    }
    return 1;
}

static int lqtL_indexfunc (lua_State *L) {
    int i = 1;
    if (lua_isuserdata(L, 1) && !lua_islightuserdata(L, 1)) {
        lua_getmetatable(L, 1);
        lua_pushliteral(L, "__get");
        lua_rawget(L, -2);
        if (lua_istable(L, -1)) {
            lua_pushvalue(L, 2);
            lua_gettable(L, -2);
            if (lua_isfunction(L, -1)) {
                lua_CFunction getter = lua_tocfunction(L, -1);
                if (!getter) return luaL_error(L, "Invalid getter %s", lua_tostring(L, 2));
                return getter(L);
            }
        }
        lua_settop(L, 2);
        lua_getfenv(L, 1); // (1)
        lua_pushvalue(L, 2); // (2)
        lua_gettable(L, -2); // (2)
        if (!lua_isnil(L, -1)) {
            lua_remove(L, -2);
            return 1;
        }
        lua_pop(L, 2); // (0)
    }
    lua_pushnil(L); // (+1)
    while (!lua_isnone(L, lua_upvalueindex(i))) { // (+1)
        lua_pop(L, 1); // (+0)
        lua_pushvalue(L, 2); // (+1)
        if (i==1) {
            lua_rawget(L, lua_upvalueindex(i)); // (+1)
        } else {
            lua_gettable(L, lua_upvalueindex(i)); // (+1)
        }
        if (!lua_isnil(L, -1)) break;
        i++;
    }
    return 1; // (+1)
}

static int lqtL_pushindexfunc (lua_State *L, const char *name, lqt_Base *bases) {
    int upnum = 1;
    luaL_newmetatable(L, name); // (1)
    while (bases->basename!=NULL) {
        luaL_newmetatable(L, bases->basename); // (upnum)
        upnum++;
        bases++;
    }
    lua_pushcclosure(L, lqtL_indexfunc, upnum); // (1)
    return 1;
}

static int lqtL_ctor_helper(lua_State*L) {
    lua_getfield(L, 1, "new");
    lua_replace(L, 1);
    lua_call(L, lua_gettop(L)-1, LUA_MULTRET);
    return lua_gettop(L);
}

static int lqtL_local_ctor(lua_State*L) {
    lua_pushvalue(L, lua_upvalueindex(1)); // (+1)
    lua_getfield(L, -1, "new"); // (+2)
    lua_insert(L, 1); // (+2)
    lua_pop(L, 1); // (+1)
    lua_call(L, lua_gettop(L)-1, LUA_MULTRET); // (X)
    lua_getfield(L, 1, "delete"); // (X+1)
    lua_setfield(L, 1, "__gc"); // (X)
    return lua_gettop(L);
}

int lqtL_createclass (lua_State *L, const char *name, luaL_Reg *mt, luaL_Reg *getters, luaL_Reg *setters, lqt_Base *bases) {
    int len = 0;
    char *new_name = NULL;
    lqt_Base *bi = bases;
    luaL_newmetatable(L, name); // (1)
    luaL_register(L, NULL, mt); // (1)
    // setup offsets
    lua_pushstring(L, name); // (2) FIXME: remove
    lua_pushinteger(L, 0); // (3) FIXME: remove
    lua_settable(L, -3); // (1) FIXME: remove
    while (bi->basename!=NULL) {
        lua_pushstring(L, bi->basename); // (2) FIXME: remove
        lua_pushinteger(L, bi->offset); // (3) FIXME: remove
        lua_settable(L, -3); // (1) FIXME: remove
        bi++;
    }
    
    if (getters) {
        lua_newtable(L);
        luaL_register(L, NULL, getters);
        lua_setfield(L, -2, "__get");
    }
    if (setters) {
        lua_newtable(L);
        luaL_register(L, NULL, setters);
        lua_setfield(L, -2, "__set");
    }
    
    // set metafunctions
    lqtL_pushindexfunc(L, name, bases); // (2)
    lua_setfield(L, -2, "__index"); // (1)
    lua_pushcfunction(L, lqtL_newindexfunc); // (2)
    lua_setfield(L, -2, "__newindex"); // (1)
    lua_pushcfunction(L, lqtL_gcfunc); // (2)
    lua_setfield(L, -2, "__gc"); // (1)
    lua_pushstring(L, name);
    lua_setfield(L, -2, "__type");

    // set it as its own metatable
    lua_pushvalue(L, -1); // (2)
    lua_setmetatable(L, -2); // (1)
    lua_pop(L, 1); // (0)
    len = strlen(name);
    new_name = (char*)malloc(len*sizeof(char));
    strncpy(new_name, name, len);
    new_name[len-1] = '\0';
    luaL_register(L, new_name, mt); // (1)
    free(new_name);
    new_name = NULL;
    lua_newtable(L); // (2)
    lua_pushcfunction(L, lqtL_ctor_helper); // (3)
    lua_setfield(L, -2, "__call"); // (2)
    lua_setmetatable(L, -2); // (1)
    lua_pushvalue(L, -1); // (2)
    lua_pushcclosure(L, lqtL_local_ctor, 1); // (2)
    lua_setfield(L, -2, "new_local");
    lua_pop(L, 1); // (0)
    /*
    lua_pushlstring(L, name, strlen(name)-1); // (1)
    lua_newtable(L); // (2)
    luaL_newmetatable(L, name); // (3)
    lua_setmetatable(L, -2); // (2)
    // don't register again but use metatable
    //luaL_register(L, NULL, mt); // (2)
    lua_settable(L, LUA_GLOBALSINDEX); // (0)
    */
    return 0;
}

bool lqtL_isinteger (lua_State *L, int i) {
	if (lua_type(L, i)==LUA_TNUMBER) {
		return lua_tointeger(L, i)==lua_tonumber(L, i);
	} else {
		return false;
	}
}
bool lqtL_isnumber (lua_State *L, int i) {
    return lua_type(L, i)==LUA_TNUMBER;
}
bool lqtL_isstring (lua_State *L, int i) {
    return lua_type(L, i)==LUA_TSTRING;
}
bool lqtL_isboolean (lua_State *L, int i) {
    return lua_type(L, i)==LUA_TBOOLEAN;
}
bool lqtL_missarg (lua_State *L, int index, int n) {
    bool ret = true;
    int i = 0;
    for (i=index;i<index+n;i++) {
        if (!lua_isnoneornil(L, i)) {
            ret = false;
            break;
        }
    }
    return ret;
}

static void CS(lua_State *L) {
    std::cerr << "++++++++++" << std::endl;
    for (int i=1;i<=lua_gettop(L);i++) {
        std::cerr << luaL_typename(L, i) << " " << lua_touserdata(L, i) << std::endl;
    }
    std::cerr << "----------" << std::endl;
}

static void lqtL_ensurepointer (lua_State *L, const void *p) { // (+1)
    lqtL_getpointertable(L); // (1)
    lua_pushlightuserdata(L, const_cast<void*>(p)); // (2)
    lua_gettable(L, -2); // (2)
    if (lua_isnil(L, -1)) { // (2)
        lua_pop(L, 1); // (1)
        const void **pp = static_cast<const void**>(lua_newuserdata(L, sizeof(void*))); // (2)
        *pp = p; // (2)
        lua_newtable(L); // (3)
        lua_setfenv(L, -2); // (2)
        lua_pushlightuserdata(L, const_cast<void*>(p)); // (3)
        lua_pushvalue(L, -2); // (4)
        lua_settable(L, -4); // (2)
    } else {
        //const void **pp = static_cast<const void**>(lua_touserdata(L, -1)); // (2)
        //if (pp!=NULL) *pp = p; // (2)
    }
    // (2)
    lua_remove(L, -2); // (1)
}

void lqtL_register (lua_State *L, const void *p) { // (+0)
    lqtL_ensurepointer(L, p);
    lua_pop(L, 1);
}

void lqtL_unregister (lua_State *L, const void *p) {
    lqtL_getpointertable(L); // (1)
    lua_pushlightuserdata(L, const_cast<void*>(p)); // (2)
    lua_gettable(L, -2); // (2)
    if (lua_isuserdata(L, -1)) {
        const void **pp = static_cast<const void**>(lua_touserdata(L, -1)); // (2)
        *pp = 0;
    }
    lua_pop(L, 1); // (1)
    lua_pushlightuserdata(L, const_cast<void*>(p)); // (2)
    lua_pushnil(L); // (3)
    lua_settable(L, -3); // (1)
    lua_pop(L, 1); // (0)
}

void lqtL_pushudata (lua_State *L, const void *p, const char *name) {
    bool already = false;
    lqtL_ensurepointer(L, p); // (1)
    if (lua_getmetatable(L, -1)) {
        // (2)
        lua_pop(L, 1); // (1)
        lua_getfield(L, -1, name); // (2)
        already = lua_toboolean(L, -1) == 1; // (2)
        lua_pop(L, 1); // (1)
    } else {
        // (1)
    }
    if (!already) {
        luaL_newmetatable(L, name); // (2)
        lua_setmetatable(L, -2); // (1)
    }
    return;
}

void lqtL_passudata (lua_State *L, const void *p, const char *name) {
    lqtL_pushudata(L, p, name);
    // used only when passing temporaries - should be deleted afterwards
    lua_getfield(L, -1, "delete");
    lua_setfield(L, -2, "__gc");
    return;
}

void lqtL_copyudata (lua_State *L, const void *p, const char *name) {
    luaL_newmetatable(L, name);
    lua_pushstring(L, "new");
    lua_rawget(L, -2);
    if (lua_isnil(L, -1)) {
        std::cerr << "cannot copy " << name << std::endl;
        lua_pop(L, 2);
        lua_pushnil(L);
    } else {
        lua_remove(L, -2);
        lqtL_pushudata(L, p, name);
        if (lqtL_pcall(L, 1, 1, 0)) {
            std::cerr << "error copying " << name << std::endl;
            lua_pop(L, 1);
            lua_pushnil(L);
        }
        // Enable autodeletion for copied stuff
        lua_getfield(L, -1, "delete");
        lua_setfield(L, -2, "__gc");
    }
    return;
}

void *lqtL_toudata (lua_State *L, int index, const char *name) {
    void *ret = 0;
    if (!lqtL_testudata(L, index, name)) return 0;
    void **pp = static_cast<void**>(lua_touserdata(L, index));
    ret = *pp;
    lua_getfield(L, index, name);
    ret = (void*)(lua_tointeger(L, -1) + (char*)ret);
    lua_pop(L, 1);
    return ret;
}

void lqtL_eraseudata (lua_State *L, int index, const char *name) {
    void *ret = 0;
    if (name!=NULL && !lqtL_testudata(L, index, name)) return;
    void **pp = static_cast<void**>(lua_touserdata(L, index));
    void *p = *pp;
    *pp = 0;
    lqtL_getpointertable(L); // (1)
    lua_pushlightuserdata(L, p); // (2)
    lua_pushnil(L); // (3)
    lua_settable(L, -3); // (1)
    lua_pop(L, 1);
    return;
}

bool lqtL_testudata (lua_State *L, int index, const char *name) {
    if (!lua_isuserdata(L, index) || lua_islightuserdata(L, index)) return false;
    lua_getfield(L, index, name);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return false;
    }
    lua_pop(L, 1);
    return true;
}

const char * lqtL_pushtrace(lua_State *L) {
    lua_getglobal(L, "debug");
    lua_getfield(L, -1, "traceback");
    lua_remove(L, -2);
    lua_call(L, 0, 1);
    return lua_tostring(L, -1);
}

void lqtL_pushenum (lua_State *L, int value, const char *name) {
    lqtL_getenumtable(L);
    lua_getfield(L, -1, name);
    lua_remove(L, -2);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_pushnil(L);
        return;
    }
    lua_pushnumber(L, value);
    lua_gettable(L, -2);
    lua_remove(L, -2);
}

bool lqtL_isenum (lua_State *L, int index, const char *name) {
    bool ret = false;
    if (!lua_isstring(L, index)) return false;
    lqtL_getenumtable(L);
    lua_getfield(L, -1, name);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 2);
        return false;
    }
    lua_remove(L, -2);
    lua_pushvalue(L, index);
    lua_gettable(L, -2);
    ret = !lua_isnil(L, -1);
    lua_pop(L, 2);
    return ret;
}

int lqtL_toenum (lua_State *L, int index, const char *name) {
    int ret = -1;
    // index = LQT_TOPOSITIVE(L, index);
    lqtL_getenumtable(L); // (1)
    lua_getfield(L, -1, name); // (2)
    if (lua_isnil(L, -1)) {
        lua_pop(L, 2); //(0)
        return 0;
    }
    lua_pushvalue(L, index); // (3)
    lua_gettable(L, -2); // (3)
    if (lqtL_isinteger(L, -1)) {
        ret = lua_tointeger(L, -1); // (3)
    } else {
        ret = lua_tointeger(L, index); // (3)
    }
    lua_pop(L, 3); // (0)
    return ret;
}

int lqtL_getflags (lua_State *L, int index, const char *name) {
    int ret = 0;
    int eindex = 0;
    int i = 1;
    if (lqtL_isinteger(L, index)) return lua_tointeger(L, index);
    if (!lua_istable(L, index)) return 0;
    lqtL_getenumtable(L); // (1)
    lua_getfield(L, -1, name); // (2)
    if (!lua_istable(L, -1)) {
        // (2)
        lua_pop(L, 2);
        return 0;
    }
    // (2)
    lua_remove(L, -2); // (1)
    eindex = lua_gettop(L);
    for (i=1;;i++) { // (1)
        lua_rawgeti(L, index, i); // (2)
        if (lua_type(L, -1)!=LUA_TSTRING) {
            lua_pop(L, 1); // (1)
            break;
        } else {
            lua_gettable(L, eindex); // (2)
            ret = ret | (int)lua_tointeger(L, -1);
            lua_pop(L, 1); // (1)
        }
    }
    // (1)
    lua_pop(L, 1); // (0)
    return ret;
}

//#include <QDebug>

void lqtL_pushflags (lua_State *L, int value, const char *name) {
    int index = 1;
    lua_newtable(L); // (1) return value
    lqtL_getenumtable(L); // (2)
    lua_getfield(L, -1, name); // (3)
    lua_remove(L, -2); // (2) stack: ret, enumtable
    lua_pushnil(L); // (3) first index
    while (lua_next(L, -2) != 0) { // (4) stack: ret, enumtable, index, value
        //if (lua_isnumber(L, -2)) {
            //qDebug() << ((void*)lua_tointeger(L, -2))
                //<< ((void*)value) << (void*)(lua_tointeger(L, -2)&value)
                //<< ((lua_tointeger(L, -2)&value)==lua_tointeger(L, -2)) << lua_tostring(L, -1);
        //}
        if (lua_isnumber(L, -2) &&
                ((lua_tointeger(L, -2)&value)==lua_tointeger(L, -2))) {
            // (4) if index is the value
            lua_rawseti(L, -4, index); // (3) the string is put into the ret table
            index = index + 1; // (3) the size of the ret table has increased
        } else {
            lua_pop(L, 1); // (3) pop the value
        }
    }
    // (2) lua_next pops the vale and pushes nothing at the end of the iteration
    // (2) stack: ret, enumtable
    lua_pop(L, 1); // (1) stack: ret
    return;
}

int lqtL_touintarray (lua_State *L) {
    unsigned int *p = NULL;
    size_t i = 0;
    size_t n, nb;
    n = lua_objlen(L, -1);
    nb = (n + 1) * sizeof(unsigned int);
    p = (unsigned int *)lua_newuserdata(L, nb);
    for (i=1;i<=n;i++) {
        lua_rawgeti(L, -2, i);
        p[i-1] = lua_tointeger(L, -1);
        lua_pop(L, 1);
    }
    lua_remove(L, -2);
    p[n] = 0;
    return 1;
}

int lqtL_pcall_debug_default (lua_State *L, int narg, int nres, int err) {
    int status = 0;
    std::cerr << "entering a pcall" << std::endl;
    status = lua_pcall(L, narg, nres, err);
    std::cerr << "pcall finished with status " << status << std::endl;
    return status;
}

int lqtL_pcall_debug (lua_State *L, int narg, int nres, int err) {
    int status = 0;
    lua_getfield(L, LUA_REGISTRYINDEX, LQT_PCALL);
    lqt_PCallPtr pcall = (lqt_PCallPtr)lua_touserdata(L, -1);
    lua_pop(L, 1);
    if (pcall) {
        status = pcall(L, narg, nres, err);
    } else {
        status = lqtL_pcall_debug_default(L, narg, nres, err);
    }
    return status;
}

void lqtL_pushudatatype (lua_State *L, int index) {
    if (!lua_isuserdata(L, index) || lua_islightuserdata(L, index)) {
        lua_pushstring(L, luaL_typename(L, index));
    } else {
        lua_getfield(L, index, "__type");
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            lua_pushstring(L, luaL_typename(L, index));
        }
    }
}

const char * lqtL_getarglist (lua_State *L) {
    int args = lua_gettop(L);
    lua_checkstack(L, args * 2);
    lua_pushliteral(L, "");
    for(int i = 1; i <= args; i++) {
        lqtL_pushudatatype(L, i);
        if (i<args)
            lua_pushliteral(L, ", ");
    }
    lua_concat(L, 2*args - 1);
    return lua_tostring(L, -1);
}

const char * lqtL_source(lua_State *L, int idx) {
    static char buf[1024]; // TODO: try something better
    lua_Debug ar = {0};
    lua_pushvalue(L, idx);
    lua_getinfo(L, ">S", &ar);
    if (ar.source[0] != '@') {
        sprintf(buf, "%s", ar.source);
    } else {
        sprintf(buf, "%s %s:%d", ar.name, ar.source, ar.linedefined);
    }
    return buf;
}

bool lqtL_is_super(lua_State *L, int idx) {
    lua_getfield(L, LUA_GLOBALSINDEX, LQT_SUPER);
    void *super = lua_touserdata(L, -1);
    void *comp = lua_touserdata(L, idx);
    bool ret = lua_equal(L, -1, idx);
    lua_pop(L, 1);
    return ret;
}

void lqtL_register_super(lua_State *L) {
    lua_getfield(L, LUA_GLOBALSINDEX, LQT_SUPER);
    if (lua_isnil(L, -1)) {
        void *ud = lua_newuserdata(L, sizeof(int));
        lua_setfield(L, LUA_GLOBALSINDEX, LQT_SUPER);
    } else {
        lua_pop(L, -1);
    }
}

// returns true if the value at index `n` can be converted to `to_type`
bool lqtL_canconvert(lua_State *L, int n, const char *to_type) {
    if (lqtL_testudata(L, n, to_type))
        return true;
    int oldtop = lua_gettop(L);
    luaL_getmetatable(L, to_type);
    lua_getfield(L, -1, "__test");
    if (lua_isnil(L, -1)) {
        lua_settop(L, oldtop);
        return false;
    }
    lqt_testfunc func = (lqt_testfunc) lua_touserdata(L, -1);
    lua_settop(L, oldtop);
    return func(L, n);
}

// converts the value at index `n` to `to_type` and returns a pointer to it
void *lqtL_convert(lua_State *L, int n, const char *to_type) {
    if (lqtL_testudata(L, n, to_type))
        return lqtL_toudata(L, n, to_type);
    int oldtop = lua_gettop(L);
    luaL_getmetatable(L, to_type);
    lua_getfield(L, -1, "__convert");
    if (lua_isnil(L, -1)) {
        lua_settop(L, oldtop);
        return false;
    }
    lqt_convertfunc func = (lqt_convertfunc) lua_touserdata(L, -1);
    lua_settop(L, oldtop);
    return func(L, n);
}

void lqtL_selfcheck(lua_State *L, void *self, const char *name) {
  if (NULL==self) {
    lua_pushfstring(L, "Instance of %s has already been deleted in:\n", name);
    lqtL_pushtrace(L);
    lua_concat(L, 2);
    lua_error(L);
  }
}
