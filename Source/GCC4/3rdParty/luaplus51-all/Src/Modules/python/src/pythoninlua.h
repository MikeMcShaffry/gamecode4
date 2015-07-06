/*

 Lunatic Python
 --------------
 
 Copyright (c) 2002-2005  Gustavo Niemeyer <gustavo@niemeyer.net>

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
#ifndef PYTHONINLUA_H
#define PYTHONINLUA_H

#define POBJECT "PyObject"

int py_convert(lua_State *L, PyObject *o, int withnone);

typedef struct {
	PyObject *o;
	int asindx;
} py_object;

void stackDump(lua_State *L);
void tableDump(lua_State *L, int t);

py_object *check_py_object(lua_State *L, int ud);

LUA_API int luaopen_python(lua_State *L);

#endif
