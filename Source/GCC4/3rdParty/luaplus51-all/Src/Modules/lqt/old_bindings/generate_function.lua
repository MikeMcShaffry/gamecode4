#!/usr/bin/lua

--[[

Copyright (c) 2007-2008 Mauro Iazzi

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

--]]

HEADER_DEFINE = tostring(os.getenv'HEADER_DEFINE' or '__LQT_FUNCTION')
ARG_MAX = tonumber(os.getenv'ARG_MAX' or 2)
TYPES = { bool='lua_pushboolean', int='lua_pushinteger', double='lua_pushnumber', ['const char *']='lua_pushstring' }

cpp = {
  string = '',
  write = function (s)
    cpp.string = cpp.string .. tostring(s)
  end,
}

hpp = {
  string = '',
  write = function (s)
    hpp.string = hpp.string .. tostring(s)
  end,
}

hpp.write("#ifndef "..HEADER_DEFINE.."\n")
hpp.write("#define "..HEADER_DEFINE.."\n")
cpp.write('#include "lqt_function.hpp"\n ')


hpp.write[[

#include "lqt_common.hpp"

#include <QObject>
//#include <QDebug>

#define LUA_FUNCTION_REGISTRY "Registry Function"
//#ifndef SEE_STACK
//# define SEE_STACK(L, j) for (int j=1;j<=lua_gettop(L);j++) { qDebug() << j << '=' << luaL_typename(L, j) << '@' << lua_topointer (L, j); }
//#endif

class LuaFunction: public QObject {
  Q_OBJECT

  public:
  LuaFunction(lua_State *state);
  virtual ~LuaFunction();

  private:
    lua_State *L;
    static int __gc (lua_State *L);
  protected:
  public:
  public slots:
]]
cpp.write[[
LuaFunction::LuaFunction(lua_State *state):L(state) {
	int functionTable = lua_gettop(L); // not yet but soon
	//qDebug() << "Function" << this << "is born";
	lua_getfield(L, LUA_REGISTRYINDEX, LUA_FUNCTION_REGISTRY);
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		lua_newtable(L);
		lua_pushvalue(L, -1);
		lua_setfield(L, LUA_REGISTRYINDEX, LUA_FUNCTION_REGISTRY);
	}
	lua_insert(L, -2);

	lua_pushvalue(L, -1);
	lua_gettable(L, functionTable);

	if (!lqtL_testudata(L, -1, "QObject*")) {
		//qDebug() << "not QObject* is" << luaL_typename(L, -1);
		lua_pop(L, 1);
		// top of stack is the function I want
		//qDebug() << "to be bound is" << luaL_typename(L, -1);
		lua_pushlightuserdata(L, this);
		lua_pushvalue(L, -2);
		lua_settable(L, functionTable);
		// registry this is associated to this function
		lqtL_passudata(L, this, "QObject*");
		lua_insert(L, -2);
		lua_pushvalue(L, -2);
		lua_settable(L, functionTable);
	} else {
		// leave the qobject on top;
		//qDebug() << "Function" << this << "scheduled for deletion";
		this->deleteLater();
	}
	lua_replace(L, functionTable);
	lua_settop(L, functionTable);
}
LuaFunction::~LuaFunction() {
	//qDebug() << "Function" << this << "is dead";
	lua_getfield(L, LUA_REGISTRYINDEX, LUA_FUNCTION_REGISTRY);
	lua_pushlightuserdata(L, this);
	lua_gettable(L, -2);
	lua_pushnil(L);
	lua_settable(L, -3);
	lua_pushlightuserdata(L, this);
	lua_pushnil(L);
	lua_settable(L, -3);
	lua_pop(L, 1);
}
]]
--[[
int LuaFunction::__gc (lua_State *L) {
  QPointer<QObject> *qp = (QPointer<QObject> *)lua_touserdata(L, 1);
  if (*qp) {
    (*qp)->deleteLater();
  }
    qDebug() << "LuaFunction" << *qp << "scheduled for deletion";
    return 0;
  }
]]

signatures = {
  [0] = {
    [''] = '',
  }
}

for nargs = 1,ARG_MAX do
  signatures[nargs] = {}
  for oldsig, oldbody in pairs(signatures[nargs-1]) do
    for argtype, argpush in pairs(TYPES) do
      signatures[nargs][oldsig..((nargs==1) and '' or ', ')..argtype..' arg'..tostring(nargs)] = oldbody..argpush.."(L, arg"..tostring(nargs)..');'
    end
  end
end

for i=0,ARG_MAX do
  for s, b in pairs(signatures[i]) do
    hpp.write('  void function ('..s..');\n')
    cpp.write'void LuaFunction::function ('
    cpp.write(s)
    cpp.write[[) {
  int functionTable = lua_gettop(L) + 1;
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_FUNCTION_REGISTRY);
  if (!lua_istable(L, -1)) {
    return;
  }
  lua_pushlightuserdata(L, this);
  lua_gettable(L, functionTable);
  ]]
    cpp.write(b)
    cpp.write'\n  '
    cpp.write'lua_call(L,'
    cpp.write(tostring(i))
    cpp.write', 0);\n'
    cpp.write[[
};]]
    cpp.write'\n'
  end
end

hpp.write[[
};


]]
hpp.write("#endif // "..HEADER_DEFINE.."\n")


io.write(hpp.string)

