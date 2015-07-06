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

#include "lqt_qt_utils.hpp"
#include "lqt_common.hpp"
#include "lqt_function.hpp"

static int qt_slot_from_string (lua_State *L) {
	lua_pushstring(L, "1");
	lua_pushvalue(L, 1);
	lua_concat(L, 2);
	return 1;
}

static int qt_signal (lua_State *L) {
	lua_pushstring(L, "2");
	lua_pushvalue(L, 1);
	lua_concat(L, 2);
	return 1;
}

static int qt_derive (lua_State *L) {
	if (!lua_isuserdata(L, 1) || !lua_getmetatable(L, 1)) {
		lua_pushnil(L);
		lua_pushstring(L, "no userdata or no metatable given");
		return 2;
	}
	lua_getfield(L, -1, "__qtype");
	if (!lua_isstring(L, -1)) {
		lua_pushnil(L);
		lua_pushstring(L, "not a Qt type");
		return 2;
	}
	lua_insert(L, -2);
	lua_newtable(L);
	lua_insert(L, -3);
	lua_settable(L, -3);
	lua_newtable(L);
	lua_insert(L, -2);
	lua_setfield(L, -2, "__base");
	lua_pushcfunction(L, lqtL_index);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, lqtL_newindex);
	lua_setfield(L, -2, "__newindex");
	lua_setmetatable(L, 1);
	return 0;
}

static int qt_slot_from_function (lua_State *L) {
	lua_pushvalue(L, 1);
	LuaFunction *f = new LuaFunction(L);
	f = 0;
	lua_pushstring(L, "1function()");
	return 2;
}

static int qt_slot (lua_State *L) {
	int ret = 0;
	if (lua_type(L, 1)==LUA_TSTRING) {
		ret = qt_slot_from_string(L);
	} else if (lua_type(L, 1)==LUA_TFUNCTION) {
		ret = qt_slot_from_function(L);
	}
	return ret;
}

static int qt_pick (lua_State *L) {
	int nargs = lua_gettop(L);
	int i = 0;
	while (i++<nargs) {
		lqtL_manageudata(L, i);
	}
	return nargs;
}

static int qt_pass (lua_State *L) {
	int nargs = lua_gettop(L);
	int i = 0;
	while (i++<nargs) {
		lqtL_unmanageudata(L, i);
	}
	return nargs;
}


static luaL_Reg libqt[] = {
	{ "pick", qt_pick },
	{ "pass", qt_pass },
	{ "slot", qt_slot },
	{ "signal", qt_signal },
	{ "derive", qt_derive },
	{ 0, 0 },
};

int luaopen_qt (lua_State *L) {
	luaL_register(L, "qt", libqt);
	return 1;
}


