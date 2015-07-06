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
#include <cstring>

#define LQT_FIXEDINDEX(i)  (i<0?(1+lua_gettop(L)+i):i)
#define LQT_MAX_ARGS 50

//#include <QDebug>
#ifndef SEE_STACK
# define SEE_STACK(L, j) for (int j=1;j<=lua_gettop(L);j++) { qDebug() << j << '=' << luaL_typename(L, j) << '@' << lua_topointer (L, j); }
#endif

// TODO
//
//  - add a metatable for void* which always results in nil and possibly
//    accounts for reassigning the userdata to the right metatable.
//  - proper descending down the dependency tree for indexing, and proper
//    handling of missing binds
//


//#include <iostream>
//using namespace std;

int check_gc(lua_State*L){
	lua_newtable(L);
	lua_getglobal(L, "print");
	lua_setfield(L, -2, "__gc");
	lua_setmetatable(L, -2);
	return 0;
}

void *get_buffer(lua_State *L, size_t sz) {
#if 1
	void *ret = lua_newuserdata(L, sz);
  //check_gc(L);
	lua_pushlightuserdata(L, ret);
	lua_insert(L, -2);
	lua_settable(L, LUA_REGISTRYINDEX);
#else
	void *ret = malloc(sz);
	cout << ret << endl;
#endif
	return ret;
}

int& lqtL_tointref (lua_State *L, int i) {
  i = LQT_FIXEDINDEX(i);
	int *ret = NULL;
	ret = (int*)get_buffer(L, sizeof(int));
	*ret = lua_type(L, i)==LUA_TNUMBER?lua_tointeger(L, i):0;
	//cout << "interef " << ret << endl;
	return *ret;
}
void lqtL_pusharguments (lua_State *L, char **argv) {
	int i = 0;
	lua_newtable(L);
	for (i=0;*argv && i<LQT_MAX_ARGS;argv++,i++) {
    lua_pushstring(L, *argv);
		lua_rawseti(L, -2, i+1);
	}
	return;
}
char** lqtL_toarguments (lua_State *L, int index) {
	index = LQT_FIXEDINDEX(index);
	char **ret;
	const char *str;
	int retlen = 0;
	size_t strlen = 0;
	int i = 0;
	retlen = lua_objlen(L, index);
	ret = (char**)get_buffer(L, sizeof(char*)*(retlen+1));
	//cout << retlen << endl;
	for (i=0;i<retlen;i++) {
		lua_rawgeti(L, index, i+1);
		if (lua_isstring(L, -1)) {
			str = lua_tolstring(L, -1, &strlen);
			ret[i] = (char*)get_buffer(L, sizeof(char)*(strlen+1));
			strncpy(ret[i], str, strlen+1);
			//cout << "arg " << (void*)ret[i] << ' ' << ret[i] << endl;
		} else {
			strlen = 0;
      str = "\0";
			ret[i] = (char*)get_buffer(L, sizeof(char)*(strlen+1));
			strncpy(ret[i], str, strlen+1);
			//cout << "Zarg " << (void*)ret[i] << ' ' << ret[i] << endl;
		}
		lua_pop(L, 1);
	}
  ret[retlen] = NULL;
	//cout << "arg[] " << ret << endl;
	return ret;
}
bool lqtL_testarguments (lua_State *L, int index) {
	return (bool)lua_istable(L, index);
}

void lqtL_setvoidmetatable (lua_State *L, int i, const char *t = 0) {
	i = LQT_FIXEDINDEX(i);
	if (luaL_newmetatable(L, "void*")) {
		lua_newtable(L);
		lua_setfield(L, -2, "__index");
		lua_pushstring(L, "void*");
		lua_setfield(L, -2, "__qtype");
	}
	lua_setmetatable(L, i);
	if (t!=0) {
		lua_newtable(L);
		lua_pushstring(L, t);
		lua_setfield(L, -2, "__unknown_type");
		lua_setfenv(L, i);
	}
}

void lqtL_getpointers (lua_State *L) {
  lua_getfield(L, LUA_REGISTRYINDEX, LQT_POINTERS);
  if (!lua_istable(L, -1)) {
    //cout << "Registry Pointers created" << endl;
    lua_pop(L, 1);
    lua_newtable(L);
    lua_newtable(L);
    lua_pushstring(L, "kv");
    lua_setfield(L, -2, "__mode");
    lua_setmetatable(L, -2);
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, LQT_POINTERS);
  }
  return;
}

int lqtL_reusepointer (lua_State *L, const void *p, const char *t) {
  lqtL_getpointers(L);
  lua_pushlightuserdata(L, const_cast<void*>(p));
  lua_gettable(L, -2);
  if (lqtL_testudata(L, -1, t) && (p==*(const void**)lua_touserdata(L, -1))) {
		lua_remove(L, -2);
		//qDebug();
		//qDebug() << "reused" << p << "in" << lua_touserdata(L, -1) << t;
		//lua_getmetatable(L, -1);luaL_getmetatable(L, t);
		//qDebug() << "meta" << lua_topointer(L, -2) << "type" << lua_topointer(L, -1);
		//lua_pop(L, 2);
    return 1;
  }
  lua_pop(L, 2);
  return 0;
}

void lqtL_pushpointer (lua_State *L, const void *obj, const char *t) {
  const void **objp = (const void**)lua_newuserdata(L, sizeof(const void *));
  *objp = 0;
  lua_getfield(L, LUA_REGISTRYINDEX, t);
  if (lua_istable(L, -1)) {
		lua_setmetatable(L, -2);
		lua_newtable(L);
		lua_setfenv(L, -2);
  } else {
    //cout << "NO metatable given" << endl;
    // TODO: add a fallback?
    lua_pop(L, 1);
		lqtL_setvoidmetatable(L, -1, t);
  }
  *objp = obj;
  //cout << "pushed " << objp << ' ' << obj << endl;
}

void lqtL_setpointer (lua_State *L, int i, const void *obj) {
  int index = i<0?(1+lua_gettop(L)+i):i;
  //cout << lua_gettop(L) << ' ' << i << ' ' << index << endl;
  lqtL_getpointers(L);
  lua_pushlightuserdata(L, const_cast<void *>(obj));
  lua_pushvalue(L, index);
  lua_settable(L, -3);
  lua_pop(L, 1);
  return;
}
void lqtL_unsetpointer (lua_State *L, const void *obj) {
  lqtL_getpointers(L);
  lua_pushlightuserdata(L, const_cast<void *>(obj));
  lua_pushnil(L);
  lua_settable(L, -3);
  lua_pop(L, 1);
  return;
}

// TODO: don't need, it's debug
void * lqtL_topointer (lua_State *L, int i) {
  void * ret = NULL;
  ret = lua_touserdata(L, i);
  ret = (ret==0)?0:*static_cast<void**>(ret);
  return ret;
}

void lqtL_unmanageudata (lua_State *L, int i) {
  if (!lua_isuserdata(L, i)) return;
  lua_getfenv(L, i);
  if (lua_istable(L, -1)) {
    lua_pushnil(L);
    lua_setfield(L, -2, "__gc");
  }
  lua_pop(L, 1);
  return;
}
void lqtL_manageudata (lua_State *L, int index) {
	//luaL_checkstack(L, 20, "");
  index = LQT_FIXEDINDEX(index);
  if (!lua_isuserdata(L, index)) return;
  lua_getfield(L, index, "delete");
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 1);
    return;
  }
  lua_getfenv(L, index);
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
  //if (1) {
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setfenv(L, index);
  }
  lua_insert(L, -2);
  lua_setfield(L, -2, "__gc");
  lua_pop(L, 1);
  return;
}
void lqtL_pushudata (lua_State *L, const void *obj, const char *t) {
  // TODO: make the udata unique
  //cout << endl << "pushing udata " << obj << " with type " << t << endl;

  lua_checkstack(L, 5);
  if (1) {
    if (lqtL_reusepointer(L, obj, t)) {
      //cout << obj << " reused " << t  << endl;
      return;
    }
		//cout << obj << " NOT reused " << t  << endl;
  } else {
    lqtL_getpointers(L);
    lua_pushlightuserdata(L, const_cast<void *>(obj));
    lua_gettable(L, -2);
    if (0) {
      //cout << lua_gettop(L) << endl;
      lua_getglobal(L, "print");
      if (lua_getmetatable(L, -2)) {
        //cout << "metatable "; // << lua_tostring(L, -1) << endl << endl;
        lua_call(L, 1, 0);
      } else {
        //cout << "no metatable" << endl;
        lua_pop(L, 1);
      }
      //cout << lua_gettop(L) << endl;
      lua_getglobal(L, "print");
      lua_getfield(L, LUA_REGISTRYINDEX, t);
      //cout << "registry  "; // << lua_tostring(L, -1) << endl; lua_pop(L, 1);
      lua_call(L, 1, 0);
    } else if (0) {
      //cout << lua_gettop(L) << endl;
      if (lua_getmetatable(L, -1)) {
        lua_getfield(L, LUA_REGISTRYINDEX, t);
        //cout << (bool)lua_equal(L, -1, -2) << endl;
        lua_getglobal(L, "print");
        lua_insert(L, -3);
        lua_call(L, 2, 0);
      } else {
        //cout << "no metatable" << endl;
        lua_pop(L, 1);
      }
    }
    if (lqtL_testudata(L, -1, t)) {
      //cout << obj << " reused" << endl;
      lua_remove(L, -2);
      return;
    } else {
      //cout << luaL_typename(L, -1) << " testudata failed " << lua_touserdata(L, -1) << ' ' << lqtL_topointer(L, -1) << endl;
    }
    lua_pop(L, 2);
  }

  if (1) {
    lqtL_pushpointer(L, obj, t);
  } else {
    const void **objp = (const void**)lua_newuserdata(L, sizeof(const void *));
    *objp = 0;
    lua_getfield(L, LUA_REGISTRYINDEX, t);
    if (lua_istable(L, -1)) {
      lua_setmetatable(L, -2);
    } else {
      //cout << "NO metatable given" << endl;
      lua_pop(L, 1);
      // TODO: add a fallback?
    }
    *objp = obj;
    //cout << "pushed " << objp << ' ' << obj << endl;
  }
  //cout << lua_gettop(L) << endl;
  if (1) {
    lqtL_setpointer(L, -1, obj);
  } else {
    lua_getfield(L, LUA_REGISTRYINDEX, LQT_POINTERS);
    lua_pushlightuserdata(L, const_cast<void *>(obj));
    lua_pushvalue(L, -3);
    lua_settable(L, -3);
    lua_pop(L, 1);
  }
  //cout << lua_gettop(L) << endl;
}
void lqtL_passudata (lua_State *L, const void *obj, const char *t) {
	//cout << "passing: " << obj << " " << t << endl;
  lqtL_pushudata(L, obj, t);
  lqtL_manageudata(L, -1);
}


int lqtL_derivesfrom (lua_State *L, int i, const char *t) {
	int ret = 0;
	int oldtop = lua_gettop(L);
	i = LQT_FIXEDINDEX(i);

  if (!lua_istable(L, i)) {
		return 0;
	}

	luaL_checkstack(L, 1, "cannot grow stack for checking object type");

	//qDebug() << "given a table" << lua_topointer(L, i);
	lua_getfield(L, LUA_REGISTRYINDEX, t);
	//qDebug() << "table" << t << lua_topointer(L, -1);
	ret = lua_equal(L, i, -1);
	//qDebug() << "same as" << t << "?" << (bool)ret;

	/*
	lua_settop(L, oldtop);

	return ret;
	// */

	if (!ret) {
		lua_pop(L, 1);
		luaL_checkstack(L, 13, "cannot grow stack for checking object type"); // FIXME: is it enough?
		lua_getfield(L, i, "__base");
    
		if (!lua_istable(L, -1)) {
			lua_pop(L, 1);
			ret = 0;
		} else {
			lua_getfield(L, -1, t);
			ret = lua_isnil(L, -1)?0:1;
			//if (ret) qDebug() << (lua_toboolean(L, -1)?"directly":"INDIRECTLY") << "derives from" << t;
			// DANGER: order of control expression is important
			while ((ret == 0) && (lua_next(L, -2) != 0)) {
				if (!lua_istable(L, -1)) {
					lua_pop(L, 1);
					lua_pushvalue(L, -1);
					lua_gettable(L, LUA_REGISTRYINDEX);
					if (lua_istable(L, -1)) {
						lua_pushvalue(L, -2);
						lua_pushvalue(L, -2);
						lua_settable(L, -5);
					}
				}
				if (lua_istable(L, -1)) {
					ret = lqtL_derivesfrom(L, -1, t);
				}
				lua_pop(L, 1);
			}
		}
	}

	lua_settop(L, oldtop);

	return ret;
}

bool lqtL_testudata (lua_State *L, int i, const char *t) {
	//qDebug() << "================ testudata" << t;
	//luaL_checkstack(L, 99, "");
	i = LQT_FIXEDINDEX(i);
	bool ret = false;
  if (lua_getmetatable(L, i)) {
		//SEE_STACK(L, pippo);
		//SEE_STACK(L, pippo);
		if (0) {
		lua_getfield(L, LUA_REGISTRYINDEX, "QEvent*");
		if (0 && lua_istable(L, -1)) {
			lua_getfield(L, -1, "__qtype");
      //SEE_STACK(L, pippo);
			lua_pop(L, 1);
		}
		lua_pop(L, 2);
		}
    ret = (bool) lqtL_derivesfrom(L, -1, t);

		//qDebug() << "derives?" << ret;
		if (!ret && lqtL_derivesfrom(L, -1, "void*")) {
			//cout << "checking for a generic void* pointer" << endl;
			//*
			luaL_checkstack(L, 3, "cannot check void* real type");
      lua_getfenv(L, i);
			lua_getfield(L, -1, "__unknown_type");
			//TODO: assign dynamically?
			lua_pushstring(L, t);
			ret = (bool)lua_equal(L, -1, -2);
			lua_pop(L, 3);
			// */
			// FIXME: deleting makes QMetaObjects not work
			//ret = true;
		}
		lua_pop(L, 1);
	} else {
	}
	//cout << t << (ret?"":" NOT") <<" found" << endl;
	return ret;
	/*
  bool ret = false;
	int oldtop = lua_gettop(L);
	lua_getfield(L, LUA_REGISTRYINDEX, t);
  if (lua_getmetatable(L, i)) {
    ret = (bool)lua_equal(L, -1, -2);
		if (!ret) {
			lua_remove(L, -2);
			ret = lqTL_derivesfrom(L, -1, t);
		}
  }
	lua_settop(L, oldtop);
  return ret;
	*/
}
//*

int lqtL_pushtype (lua_State *L, int i) {
	int type = lua_type(L, i);
	if (type != LUA_TUSERDATA) {
		lua_pushstring(L, lua_typename(L, type));
	} else {
    lua_getfield(L, i, "__qtype");
		if (!lua_isstring(L, -1)) {
			lua_pop(L, 1);
			lua_pushstring(L, "<unknown>");
		}
	}
	return 1;
}

void * lqtL_checkudata (lua_State *L, int i, const char *t) {
	if (lqtL_testudata(L, i, t)) {
		return lua_touserdata(L, i);
	} else {
		lua_pushstring(L, "Fatal error: userdata type mismatch. requested ");
		lua_pushstring(L, t);
		lua_pushstring(L, " found ");
    lqtL_pushtype(L, i);
		lua_concat(L, 4);
		lua_error(L);
	}
	return 0;
}
//*/
void * lqtL_toudata (lua_State *L, int i, const char *t) {
  void * ret = NULL;
  if (lqtL_testudata(L, i, t)) {
    ret = *static_cast<void**>(lua_touserdata(L, i));
  }
  return ret;
}

void lqtL_pushenum (lua_State *L, int v, const char *e) {
  lua_getfield(L, LUA_REGISTRYINDEX, LQT_ENUMS);
  if (lua_istable(L, -1)) {
		//qDebug() << "LQT_ENUMS is a table";
    lua_getfield(L, -1, e);
    lua_remove(L, -2);
  } else {
		//qDebug() << "LQT_ENUMS is NOT a table";
	}
  lua_pushinteger(L, v);
  if (lua_istable(L, -2)) {
		//qDebug() << "getting translation";
    lua_gettable(L, -2);
  } else {
		//qDebug() << "no translation table for" << e;
	}
  lua_remove(L, -2);
}
bool lqtL_isenum (lua_State *L, int i, const char *e) {
  bool ret = false;
  if (lua_type(L, i)==LUA_TNUMBER) {
    ret = (lua_tointeger(L, i)==lua_tonumber(L, i));
  } else if (lua_type(L, i)==LUA_TSTRING) {
    lua_getfield(L, LUA_REGISTRYINDEX, LQT_ENUMS);
    if (lua_istable(L, -1)) {
      lua_getfield(L, -1, e);
      lua_remove(L, -2);
    }
    lua_pushvalue(L, i);
    if (lua_istable(L, -2)) {
      lua_gettable(L, -2);
    }
    if (lua_type(L, -1)==LUA_TNUMBER) {
      ret = (lua_tointeger(L, -1)==lua_tonumber(L, -1));
    }
    lua_pop(L, 2);
  }
  return ret;
}
int lqtL_toenum (lua_State *L, int i, const char *e) {
  int ret = -1;
  if (lua_type(L, i)==LUA_TNUMBER) {
    ret = lua_tointeger(L, i);
  } else if (lua_type(L, i)==LUA_TSTRING) {
    lua_getfield(L, LUA_REGISTRYINDEX, LQT_ENUMS);
    if (lua_istable(L, -1)) {
      lua_getfield(L, -1, e);
      lua_remove(L, -2);
    }
    lua_pushvalue(L, i);
    if (lua_istable(L, -2)) {
      lua_gettable(L, -2);
    }
    if (lua_type(L, -1)==LUA_TNUMBER) {
      ret = lua_tointeger(L, -1);
    }
    lua_pop(L, 2);
  }
  return ret;
}

int lqtL_baseindex (lua_State *L, int index, int key) {
	int ret = 0;
	int oldtop = lua_gettop(L);
	index = LQT_FIXEDINDEX(index);
	key = LQT_FIXEDINDEX(key);

  if (!lua_istable(L, index)) {
		return 0;
	}

	luaL_checkstack(L, 1, "cannot grow stack for retrieving member");

	lua_pushvalue(L, key);
	lua_gettable(L, index);

	if (!lua_isnil(L, -1)) {
		ret = 1;
	} else {
		luaL_checkstack(L, 7, "cannot grow stack for retrieving member"); // FIXME: is it enough?
		lua_getfield(L, index, "__base");
    
		if (!lua_istable(L, -1)) {
			lua_pop(L, 2);
			ret = 0;
		} else {
			lua_insert(L, -2);
			// DANGER: order of control expression is important
			while ((ret == 0) && (lua_next(L, -2) != 0)) {
				if (!lua_istable(L, -1) && lua_isboolean(L, -1) && lua_toboolean(L, -1)) {
					lua_pop(L, 1);
					lua_pushvalue(L, -1);
					lua_gettable(L, LUA_REGISTRYINDEX);
					if (lua_istable(L, -1)) {
						lua_pushvalue(L, -2);
						lua_pushvalue(L, -2);
						lua_settable(L, -5);
					}
				}
				if (lua_istable(L, -1)) {
					ret = lqtL_baseindex(L, -1, key);
				}
				if (ret == 0) {
					lua_pop(L, 1);
				}
			}
		}
	}

	if (ret != 0) {
    lua_insert(L, oldtop+1);
	}
	lua_settop(L, oldtop+ret);

	return ret;
}

int lqtL_index (lua_State *L) {
	lua_settop(L, 2);
	luaL_checkstack(L, 1, "cannot grow stack for retrieving member");
	if (!lua_getmetatable(L, 1)) {
		return 0;
	}

	return lqtL_baseindex(L, 3, 2);
/*
	luaL_checkstack(L, 2, "cannot grow stack for retrieving member");
  lua_getmetatable(L, 1);
  if (lua_istable(L, -1)) {
    lua_pushvalue(L, 2);
    lua_gettable(L, -2);
  } else {
    lua_pushnil(L);
  }
  lua_remove(L, -2);
  return 1;
*/
}

int lqtL_newindex (lua_State *L) {
	if (!lua_getmetatable(L, 1)) {
		return 0;
	}
  if (lua_istable(L, -1)) {
    lua_pushvalue(L, 2);
    lua_pushvalue(L, 3);
    lua_settable(L, -3);
    lua_pop(L, 1);
  }
  return 0;
}

int lqtL_gc (lua_State *L) {
  if (!lua_isuserdata(L, 1)) return 0;

  lua_getfenv(L, 1);
  if (lua_istable(L, -1)) {
    lua_getfield(L, -1, "__gc");
    if (lua_isfunction(L, -1)) {
      //cout << "found fenv gc " << lua_topointer(L, -1) << endl;
      lua_pushvalue(L, 1);
      lua_call(L, 1, 0);
    } else {
      //cout << "NOT found fenv gc" << endl;
      lua_pop(L, 1);
    }
  }
  lua_pop(L, 1);

  // FIXME: this is useless, isn't it?
  void **p = static_cast<void**>(lua_touserdata(L, 1));
  *p = 0;

  return 0;
}


