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
#include <Python.h>

#include <lua.h>
#include <lauxlib.h>

#include "pythoninlua.h"
#include "luainpython.h"

static int py_asfunc_call(lua_State *L);

void stackDump(lua_State *L)
{
	int i;
	int top = lua_gettop(L);
	for (i = 1; i <= top; i++) {
		int t = lua_type(L, i);
		switch (t) {
		case LUA_TSTRING:
			fprintf(stderr, "'%s'", lua_tostring(L, i));
			break;
		case LUA_TBOOLEAN:
			fprintf(stderr, lua_toboolean(L,i) ? "true" : "false");
			break;
		case LUA_TNUMBER:
			fprintf(stderr, "%lg", lua_tonumber(L, i));
			break;
		default:
			fprintf(stderr, "%s<%p>", lua_typename(L, t),
			       lua_topointer(L, i));
			break;
		}
		fprintf(stderr, "  ");
	}
	fprintf(stderr, "\n");
	fflush(stderr);
}

void tableDump(lua_State *L, int t)
{
	if (!lua_istable(L, t)) {
		luaL_typerror(L, t, "not a table");
	}
	lua_pushnil(L);
	while (lua_next(L, t) != 0) {
		// key value
		lua_getfield(L, LUA_GLOBALSINDEX, "tostring");
		// key value <tostring>
		lua_pushvalue(L, -3);
		// key value <tostring> key
		lua_call(L, 1, 1);
		// key value "key"
		lua_getfield(L, LUA_GLOBALSINDEX, "tostring");
		// key value "key" <tostring>
		lua_pushvalue(L, -3);
		// key value "key" <tostring> value
		lua_call(L, 1, 1);
		// key value "key" "value"
		fprintf(stderr, "%s - %s\n",
			lua_tostring(L, -2), lua_tostring(L, -1));
		lua_pop(L, 3);
	}
}

/* Replacement for luaL_checkudata that doesn't throw an error */
py_object* check_py_object(lua_State *L, int ud)
{
	void *p = lua_touserdata(L, ud);
	if (p != NULL) {	/* value is a userdata? */
		if (lua_getmetatable(L, ud)) { /* does it have a metatable? */
			lua_getfield(L, LUA_REGISTRYINDEX, POBJECT);
			if (lua_rawequal(L, -1, -2)) {
				lua_pop(L, 2);
				return p;
			}
		}
	}
	return NULL;
}

static int py_convert_custom(lua_State *L, PyObject *o, int asindx)
{
	int ret = 0;
	py_object *obj = (py_object*) lua_newuserdata(L, sizeof(py_object));
	if (obj) {
		Py_INCREF(o);
		obj->o = o;
		obj->asindx = asindx;
		luaL_getmetatable(L, POBJECT);
		lua_setmetatable(L, -2);
		ret = 1;
	} else {
		luaL_error(L, "failed to allocate userdata object");
	}
	return ret;
}

int py_convert(lua_State *L, PyObject *o, int withnone)
{
	int ret = 0;
	if (o == Py_None) {
		if (withnone) {
			lua_pushliteral(L, "Py_None");
			lua_rawget(L, LUA_REGISTRYINDEX);
			if (lua_isnil(L, -1)) {
				lua_pop(L, 1);
				luaL_error(L, "lost none from registry");
			}
			ret = 1;
		} else {
			/* Not really needed, but this way we may check
			 * for errors with ret == 0. */
			lua_pushnil(L);
			ret = 1;
		}
	} else if (o == Py_True) {
		lua_pushboolean(L, 1);
		ret = 1;
	} else if (o == Py_False) {
		lua_pushboolean(L, 0);
		ret = 1;
	} else if (PyString_Check(o)) {
		char *s;
		Py_ssize_t len;
		if (PyString_AsStringAndSize(o, &s, &len) < 0) {
			luaL_error(L, "failed string conversion");
		}
		lua_pushlstring(L, s, len);
		ret = 1;
	} else if (PyInt_Check(o)) {
		lua_pushinteger(L, PyInt_AsLong(o));
		ret = 1;
	} else if (PyFloat_Check(o)) {
		lua_pushnumber(L, PyFloat_AsDouble(o));
		ret = 1;
	} else if (LuaObject_Check(o)) {
		lua_rawgeti(L, LUA_REGISTRYINDEX, ((LuaObject*)o)->ref);
		ret = 1;
	} else {
		int asindx = 0;
		if (PyDict_Check(o) || PyList_Check(o) || PyTuple_Check(o))
			asindx = 1;
		ret = py_convert_custom(L, o, asindx);
		if (ret && !asindx &&
		    (PyFunction_Check(o) || PyCFunction_Check(o)))
			lua_pushcclosure(L, py_asfunc_call, 1);
	}
	return ret;
}

static int py_object_call(lua_State *L)
{
	py_object *obj = check_py_object(L, 1);
	PyObject *args;
	PyObject *value;
	int nargs = lua_gettop(L)-1;
	int ret = 0;
	int i;

	if (!obj) {
		luaL_argerror(L, 1, "not a python object");
		return 0;
	}
	if (!PyCallable_Check(obj->o)) {
		luaL_error(L, "object is not callable");
		return 0;
	}

	args = PyTuple_New(nargs);
	if (!args) {
                PyErr_Print();
		luaL_error(L, "failed to create arguments tuple");
		return 0;
	}
	
	for (i = 0; i != nargs; i++) {
		PyObject *arg = LuaConvert(L, i+2);
		if (!arg) {
			luaL_error(L, "failed to convert argument #%d", i+1);
			Py_DECREF(args);
			return 0;
		}
		PyTuple_SetItem(args, i, arg);
	}

	value = PyObject_CallObject(obj->o, args);
	if (value) {
		ret = py_convert(L, value, 0);
		Py_DECREF(value);
	} else {
		PyErr_Print();
		luaL_error(L, "error calling python function");
	}
	
	return ret;
}

static int _p_object_newindex_set(lua_State *L, py_object *obj,
				  int keyn, int valuen)
{
	PyObject *value;
	PyObject *key = LuaConvert(L, keyn);

	if (!key) {
		luaL_argerror(L, 1, "failed to convert key");
		return 0;
	}

	if (!lua_isnil(L, valuen)) {
		value = LuaConvert(L, valuen);
		if (!value) {
			Py_DECREF(key);
			luaL_argerror(L, 1, "failed to convert value");
			return 0;
		}

		if (PyObject_SetItem(obj->o, key, value) == -1) {
			PyErr_Print();
			luaL_error(L, "failed to set item");
		}

		Py_DECREF(value);
	} else {
		if (PyObject_DelItem(obj->o, key) == -1) {
			PyErr_Print();
			luaL_error(L, "failed to delete item");
		}
	}

	Py_DECREF(key);

	return 0;
}

static int py_object_newindex_set(lua_State *L)
{
	py_object *obj = check_py_object(L, lua_upvalueindex(1));
	if (lua_gettop(L) != 2) {
		luaL_error(L, "invalid arguments");
		return 0;
	}
	return _p_object_newindex_set(L, obj, 1, 2);
}

static int py_object_newindex(lua_State *L)
{
	py_object *obj = check_py_object(L, 1);
	const char *attr;
	PyObject *value;

	if (!obj) {
		luaL_argerror(L, 1, "not a python object");
		return 0;
	}

	if (obj->asindx)
		return _p_object_newindex_set(L, obj, 2, 3);

	attr = luaL_checkstring(L, 2);
	if (!attr) {
		luaL_argerror(L, 2, "string needed");
		return 0;
	}

	value = LuaConvert(L, 3);
	if (!value) {
		luaL_argerror(L, 1, "failed to convert value");
		return 0;
	}

	if (PyObject_SetAttrString(obj->o, (char*)attr, value) == -1) {
		Py_DECREF(value);
                PyErr_Print();
		luaL_error(L, "failed to set value");
		return 0;
	}

	Py_DECREF(value);

	return 0;
}

static int _p_object_index_get(lua_State *L, py_object *obj, int keyn)
{
	PyObject *key = LuaConvert(L, keyn);
	PyObject *item;
	int ret = 0;

	if (!key) {
		luaL_argerror(L, 1, "failed to convert key");
		return 0;
	}

	item = PyObject_GetItem(obj->o, key);

	Py_DECREF(key);

	if (item) {
		ret = py_convert(L, item, 0);
		Py_DECREF(item);
	} else {
		PyErr_Clear();
		if (lua_gettop(L) > keyn) {
			lua_pushvalue(L, keyn+1);
			ret = 1;
		}
	}
	
	return ret;
}

static int py_object_index_get(lua_State *L)
{
	py_object *obj = check_py_object(L, lua_upvalueindex(1));
	int top = lua_gettop(L);
	if (top < 1 || top > 2) {
		luaL_error(L, "invalid arguments");
		return 0;
	}
	return _p_object_index_get(L, obj, 1);
}

static int py_object_index(lua_State *L)
{
	py_object *obj = check_py_object(L, 1);
	const char *attr;
	PyObject *value;
	int ret = 0;

	if (!obj) {
		luaL_argerror(L, 1, "not a python object");
		return 0;
	}

	if (obj->asindx)
		return _p_object_index_get(L, obj, 2);

	attr = luaL_checkstring(L, 2);
	if (!attr) {
		luaL_argerror(L, 2, "string needed");
		return 0;
	}

	if (attr[0] == '_' && strcmp(attr, "__get") == 0) {
		lua_pushvalue(L, 1);
		lua_pushcclosure(L, py_object_index_get, 1);
		return 1;
	} else if (attr[0] == '_' && strcmp(attr, "__set") == 0) {
		lua_pushvalue(L, 1);
		lua_pushcclosure(L, py_object_newindex_set, 1);
		return 1;
	}


	value = PyObject_GetAttrString(obj->o, (char*)attr);
	if (value) {
		ret = py_convert(L, value, 0);
		Py_DECREF(value);
	} else {
		PyErr_Clear();
		luaL_error(L, "unknown attribute in python object");
	}

	return ret;
}

static int py_object_gc(lua_State *L)
{
	py_object *obj = check_py_object(L, 1);
	if (obj) {
		Py_DECREF(obj->o);
	}
	return 0;
}

static int py_object_tostring(lua_State *L)
{
	py_object *obj = check_py_object(L, 1);
	if (obj) {
		PyObject *repr = PyObject_Str(obj->o);
		if (!repr) {
			char buf[256];
			snprintf(buf, 256, "python object: %p", obj->o);
			lua_pushstring(L, buf);
			PyErr_Clear();
		} else {
			char *s;
			Py_ssize_t len;
			PyString_AsStringAndSize(repr, &s, &len);
			lua_pushlstring(L, s, len);
			Py_DECREF(repr);
		}
	}
	return 1;
}

static const luaL_reg py_object_lib[] = {
	{"__call",	py_object_call},
	{"__index",	py_object_index},
	{"__newindex",	py_object_newindex},
	{"__gc",	py_object_gc},
	{"__tostring",	py_object_tostring},
	{NULL, NULL}
};

static int py_run(lua_State *L, int eval)
{
	const char *s;
	char *buffer = NULL;
	PyObject *m, *d, *o;
	int ret = 0;
	int len;

	s = luaL_checkstring(L, 1);
	if (!s)
		return 0;

	if (!eval) {
		len = strlen(s)+1;
		buffer = (char *) malloc(len+1);
		if (!buffer) {
			luaL_error(L, "Failed allocating buffer for execution");
			return 0;
		}
		strcpy(buffer, s);
		buffer[len-1] = '\n';
		buffer[len] = '\0';
		s = buffer;
	}

        m = PyImport_AddModule("__main__");
        if (!m) {
		free(buffer);
		luaL_error(L, "Can't get __main__ module");
		return 0;
	}
        d = PyModule_GetDict(m);

        o = PyRun_StringFlags(s, eval ? Py_eval_input : Py_single_input,
			      d, d, NULL);

	free(buffer);

        if (!o) {
                PyErr_Print();
		return 0;
        }

	if (py_convert(L, o, 0))
		ret = 1;

	Py_DECREF(o);

        if (Py_FlushLine())
                PyErr_Clear();

	return ret;
}

static int py_execute(lua_State *L)
{
	return py_run(L, 0);
}

static int py_eval(lua_State *L)
{
	return py_run(L, 1);
}

static int py_asindx(lua_State *L)
{
	py_object *obj = check_py_object(L, 1);
	if (obj)
		return py_convert_custom(L, obj->o, 1);
	else
		luaL_argerror(L, 1, "not a python object");
	return 0;

}

static int py_asattr(lua_State *L)
{
	py_object *obj = check_py_object(L, 1);
	if (obj)
		return py_convert_custom(L, obj->o, 0);
	else
		luaL_argerror(L, 1, "not a python object");
	return 0;

}

static int py_asfunc_call(lua_State *L)
{
	lua_pushvalue(L, lua_upvalueindex(1));
	lua_insert(L, 1);
	return py_object_call(L);
}

static int py_asfunc(lua_State *L)
{
	int ret = 0;
	if (check_py_object(L, 1)) {
		lua_pushcclosure(L, py_asfunc_call, 1);
		ret = 1;
	} else {
		luaL_argerror(L, 1, "not a python object");
	}
	return ret;

}

static int py_globals(lua_State *L)
{
	PyObject *globals;

	if (lua_gettop(L) != 0) {
		luaL_error(L, "invalid arguments");
		return 0;
	}

	globals = PyEval_GetGlobals();
	if (!globals) {
		PyObject *module = PyImport_AddModule("__main__");
		if (!module) {
			luaL_error(L, "Can't get __main__ module");
			return 0;
		}
		globals = PyModule_GetDict(module);
	}

	if (!globals) {
		PyErr_Print();
		luaL_error(L, "can't get globals");
		return 0;
	}

	return py_convert_custom(L, globals, 1);
}

static int py_locals(lua_State *L)
{
	PyObject *locals;

	if (lua_gettop(L) != 0) {
		luaL_error(L, "invalid arguments");
		return 0;
	}

	locals = PyEval_GetLocals();
	if (!locals)
		return py_globals(L);

	return py_convert_custom(L, locals, 1);
}

static int py_builtins(lua_State *L)
{
	PyObject *builtins;

	if (lua_gettop(L) != 0) {
		luaL_error(L, "invalid arguments");
		return 0;
	}

	builtins = PyEval_GetBuiltins();
	if (!builtins) {
		PyErr_Print();
		luaL_error(L, "failed to get builtins");
		return 0;
	}

	return py_convert_custom(L, builtins, 1);
}

static int py_import(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);
	PyObject *module;
	int ret;

	if (!name) {
		luaL_argerror(L, 1, "module name expected");
		return 0;
	}

	module = PyImport_ImportModule((char*)name);

	if (!module) {
		PyErr_Print();
		luaL_error(L, "failed importing '%s'", name);
		return 0;
	}

	ret = py_convert_custom(L, module, 0);
	Py_DECREF(module);
	return ret;
}

static const luaL_reg py_lib[] = {
	{"execute",	py_execute},
	{"eval",	py_eval},
	{"asindx",	py_asindx},
	{"asattr",	py_asattr},
	{"asfunc",	py_asfunc},
	{"locals",	py_locals},
	{"globals",	py_globals},
	{"builtins",	py_builtins},
	{"import",	py_import},
	{NULL, NULL}
};

LUA_API int luaopen_python(lua_State *L)
{
	int rc;

	/* Register module */
	luaL_register(L, "python", py_lib);

	/* Register python object metatable */
	luaL_newmetatable(L, POBJECT);
	luaL_register(L, NULL, py_object_lib);
	lua_pop(L, 1);

	/* Initialize Lua state in Python territory */
	if (!LuaState) LuaState = L;

	/* Initialize Python interpreter */
	if (!Py_IsInitialized()) {
		PyObject *luam, *mainm, *maind;
		char *argv[] = {"<lua>", 0};
		Py_SetProgramName("<lua>");
		Py_Initialize();
		PySys_SetArgv(1, argv);
		initlua();
		/* Import 'lua' automatically. */
		luam = PyImport_ImportModule("lua");
		if (!luam) {
			luaL_error(L, "Can't import lua module");
		} else {
			mainm = PyImport_AddModule("__main__");
			if (!mainm) {
				luaL_error(L, "Can't get __main__ module");
			} else {
				maind = PyModule_GetDict(mainm);
				PyDict_SetItemString(maind, "lua", luam);
				Py_DECREF(luam);
			}
		}
	}

	/* Register 'none' */
	lua_pushliteral(L, "Py_None");
	rc = py_convert_custom(L, Py_None, 0);
	if (rc) {
		lua_pushliteral(L, "none"); /* M "Py_None" None "none" */
		lua_pushvalue(L, -2);	    /* M "Py_None" None "none" None */
		lua_rawset(L, -5); /* python.none */
		lua_rawset(L, LUA_REGISTRYINDEX); /* registry.Py_None */
	} else {
		lua_pop(L, 1);
		luaL_error(L, "failed to convert none object");
	}

	return 1;
}
