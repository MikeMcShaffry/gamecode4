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
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <setjmp.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "pythoninlua.h"
#include "luainpython.h"

jmp_buf errjmp;

static int py_lua_panic(lua_State* L)
{
	size_t len;
	const char *s = lua_tolstring(L, -1, &len);
	PyObject *o = PyString_FromStringAndSize(s, len);
	PyErr_SetObject(PyExc_RuntimeError, o);
	longjmp(errjmp, -1);
	return (-1);
}

#define TRY     do {							\
        lua_CFunction old_panic = lua_atpanic(LuaState, py_lua_panic);	\
	if (setjmp(errjmp) == 0) {
#define CATCH    lua_atpanic(LuaState, old_panic);	\
        } else {					\
		  lua_atpanic(LuaState, old_panic);
#define ENDTRY   }} while (0)


lua_State *LuaState = NULL;

static PyObject *LuaObject_New(int n);

PyObject *LuaConvert(lua_State *L, int n)
{
	PyObject *ret = NULL;

	switch (lua_type(L, n)) {

		case LUA_TNIL:
			Py_INCREF(Py_None);
			ret = Py_None;
			break;

		case LUA_TSTRING: {
			size_t len;
			const char *s = lua_tolstring(L, n, &len);
			ret = PyString_FromStringAndSize(s, len);
			break;
		}

		case LUA_TNUMBER: {
			lua_Number num = lua_tonumber(L, n);
#ifdef LUA_NUMBER_DOUBLE
			if (num != (long)num) {
				ret = PyFloat_FromDouble(n);
			} else
#endif
			{
				ret = PyInt_FromLong((long)num);
			}
			break;
		}

		case LUA_TBOOLEAN:
			if (lua_toboolean(L, n)) {
				Py_INCREF(Py_True);
				ret = Py_True;
			} else {
				Py_INCREF(Py_False);
				ret = Py_False;
			}
			break;

		case LUA_TUSERDATA: {
			py_object *obj = check_py_object(L, n);

			if (obj) {
				Py_INCREF(obj->o);
				ret = obj->o;
				break;
			}

			/* Otherwise go on and handle as custom. */
		}

		default:
			ret = LuaObject_New(n);
			break;
	}

	return ret;
}

static int e_py_convert(lua_State *L, PyObject *o, int withnone)
{
	int r = 0;
	TRY {
		r = py_convert(L, o, withnone);
	} CATCH {
		r = 0;
	} ENDTRY;
	if (!r) {
		PyErr_SetString(PyExc_RuntimeError, "can't convert");
	}
	return r;
}

static PyObject *LuaCall(lua_State *L, PyObject *args)
{
	PyObject *ret = NULL;
	PyObject *arg;
	int nargs, rc, i;

	assert(PyTuple_Check(args));

	nargs = PyTuple_Size(args);
	for (i = 0; i != nargs; i++) {
		arg = PyTuple_GetItem(args, i);
		if (arg == NULL) {
			PyErr_Format(PyExc_TypeError,
				     "failed to get tuple item #%d", i);
			lua_settop(L, 0);
			return NULL;
		}
		rc = e_py_convert(L, arg, 0);
		if (!rc) {
			PyErr_Format(PyExc_TypeError,
				     "failed to convert argument #%d", i);
			lua_settop(L, 0);
			return NULL;
		}
	}

	if (lua_pcall(L, nargs, LUA_MULTRET, 0) != 0) {
		PyErr_Format(PyExc_Exception,
			     "error: %s", lua_tostring(L, -1));
		return NULL;
	}

	nargs = lua_gettop(L);
	if (nargs == 1) {
		ret = LuaConvert(L, 1);
		if (!ret) {
			PyErr_SetString(PyExc_TypeError,
				        "failed to convert return");
			lua_settop(L, 0);
			Py_DECREF(ret);
			return NULL;
		}
	} else if (nargs > 1) {
		ret = PyTuple_New(nargs);
		if (!ret) {
			PyErr_SetString(PyExc_RuntimeError,
					"failed to create return tuple");
			lua_settop(L, 0);
			return NULL;
		}
		for (i = 0; i != nargs; i++) {
			arg = LuaConvert(L, i+1);
			if (!arg) {
				PyErr_Format(PyExc_TypeError,
					     "failed to convert return #%d", i);
				lua_settop(L, 0);
				Py_DECREF(ret);
				return NULL;
			}
			PyTuple_SetItem(ret, i, arg);
		}
	} else {
		Py_INCREF(Py_None);
		ret = Py_None;
	}
	
	lua_settop(L, 0);

	return ret;
}

#define L LuaState

static PyObject *LuaObject_New(int n)
{
	LuaObject *obj = PyObject_New(LuaObject, &LuaObject_Type);
	if (obj) {
		lua_pushvalue(L, n);
		obj->ref = luaL_ref(L, LUA_REGISTRYINDEX);
		obj->refiter = LUA_NOREF;
	}
	return (PyObject*) obj;
}

static void LuaObject_dealloc(LuaObject *self)
{
	luaL_unref(L, LUA_REGISTRYINDEX, self->ref);
	if (self->refiter != LUA_NOREF)
		luaL_unref(L, LUA_REGISTRYINDEX, self->refiter);
	self->ob_type->tp_free((PyObject *)self);
}

static PyObject *LuaObject_getattr(PyObject *obj, PyObject *attr)
{
	PyObject *ret = NULL;
	int rc;
	lua_rawgeti(L, LUA_REGISTRYINDEX, ((LuaObject*)obj)->ref);
	if (lua_isnil(L, -1)) {
		PyErr_SetString(PyExc_RuntimeError, "lost reference");
		goto error;
	}
	rc = e_py_convert(L, attr, 0);
	if (rc) {
		TRY {
			lua_gettable(L, -2);
		} CATCH {
			goto error;
		} ENDTRY;

		ret = LuaConvert(L, -1);
	} else {
		PyErr_SetString(PyExc_ValueError, "can't convert attr/key");
	}
  error:
	lua_settop(L, 0);
	return ret;
}

static int LuaObject_setattr(PyObject *obj, PyObject *attr, PyObject *value)
{
	int ret = -1;
	int rc;
	lua_rawgeti(L, LUA_REGISTRYINDEX, ((LuaObject*)obj)->ref);
	if (lua_isnil(L, -1)) {
		PyErr_SetString(PyExc_RuntimeError, "lost reference");
		goto error;
	}
	if (!lua_istable(L, -1)) {
		PyErr_SetString(PyExc_TypeError, "Lua object is not a table");
		goto error;
	}
	rc = e_py_convert(L, attr, 0);
	if (rc) {
		rc = e_py_convert(L, value, 0);
		if (rc) {
			TRY {
				lua_settable(L, -3);
			} CATCH {
				ret = -1;
				goto error;
			} ENDTRY;
			ret = 0;
		} else {
			PyErr_SetString(PyExc_ValueError,
					"can't convert value");
		}
	} else {
		PyErr_SetString(PyExc_ValueError, "can't convert key/attr");
	}
  error:
	lua_settop(L, 0);
	return ret;
}

static PyObject *LuaObject_str(PyObject *obj)
{
	PyObject *ret = NULL;
	const char *s;
	int r = 0;
	lua_rawgeti(L, LUA_REGISTRYINDEX, ((LuaObject*)obj)->ref);
	TRY {
		r = luaL_callmeta(L, -1, "__tostring");
	} CATCH { 
	} ENDTRY;
	if (r) {
		s = lua_tostring(L, -1);
		lua_pop(L, 1);
		if (s) ret = PyString_FromString(s);
	}
	if (!ret) {
		int type = lua_type(L, -1);
		switch (type) {
			case LUA_TTABLE:
			case LUA_TFUNCTION:
				ret = PyString_FromFormat("<Lua %s at %p/r=%d>",
							  lua_typename(L, type),
							  lua_topointer(L, -1),
							  ((LuaObject*)obj)->ref);
				break;
			
			case LUA_TUSERDATA:
			case LUA_TLIGHTUSERDATA:
				ret = PyString_FromFormat("<Lua %s at %p>",
					lua_typename(L, type),
					lua_touserdata(L, -1));
				break;

			case LUA_TTHREAD:
				ret = PyString_FromFormat("<Lua %s at %p>",
					lua_typename(L, type),
					(void*)lua_tothread(L, -1));
				break;

			default:
				ret = PyString_FromFormat("<Lua %s>",
					lua_typename(L, type));
				break;

		}
	}
	lua_pop(L, 1);
	return ret;
}

static PyObject *LuaObject_call(PyObject *obj, PyObject *args)
{
	lua_settop(L, 0);
	lua_rawgeti(L, LUA_REGISTRYINDEX, ((LuaObject*)obj)->ref);
	return LuaCall(L, args);
}

static PyObject *LuaObject_iternext(LuaObject *obj)
{
	PyObject *ret = NULL;
	int r = 0;

	lua_rawgeti(L, LUA_REGISTRYINDEX, ((LuaObject*)obj)->ref);

	if (obj->refiter == LUA_NOREF)
		lua_pushnil(L);
	else
		lua_rawgeti(L, LUA_REGISTRYINDEX, obj->refiter);

	TRY {
		r = lua_next(L, -2);
	} CATCH {
	} ENDTRY;
	if (r != 0) {
		/* Remove value. */
		lua_pop(L, 1);
		ret = LuaConvert(L, -1);
		/* Save key for next iteration. */
		if (obj->refiter == LUA_NOREF)
			obj->refiter = luaL_ref(L, LUA_REGISTRYINDEX);
		else
			lua_rawseti(L, LUA_REGISTRYINDEX, obj->refiter);
	} else if (obj->refiter != LUA_NOREF) {
		luaL_unref(L, LUA_REGISTRYINDEX, obj->refiter);
		obj->refiter = LUA_NOREF;
	}

	return ret;
}

static Py_ssize_t LuaObject_length(LuaObject *obj)
{
	size_t len;
	lua_rawgeti(L, LUA_REGISTRYINDEX, ((LuaObject*)obj)->ref);
	len = lua_objlen(L, -1);
	lua_settop(L, 0);
	return len;
}

static PyObject *LuaObject_subscript(PyObject *obj, PyObject *key)
{
	return LuaObject_getattr(obj, key);
}

static int LuaObject_ass_subscript(PyObject *obj,
				   PyObject *key, PyObject *value)
{
	return LuaObject_setattr(obj, key, value);
}

static PyMappingMethods LuaObject_as_mapping = {
	(lenfunc)LuaObject_length,	/*mp_length*/
	(binaryfunc)LuaObject_subscript,/*mp_subscript*/
	(objobjargproc)LuaObject_ass_subscript,/*mp_ass_subscript*/
};

PyTypeObject LuaObject_Type = {
	PyObject_HEAD_INIT(NULL)
	0,			/*ob_size*/
	"lua.custom",		/*tp_name*/
	sizeof(LuaObject),	/*tp_basicsize*/
	0,			/*tp_itemsize*/
	(destructor)LuaObject_dealloc, /*tp_dealloc*/
	0,			/*tp_print*/
	0,			/*tp_getattr*/
	0,			/*tp_setattr*/
	0,			/*tp_compare*/
	LuaObject_str,		/*tp_repr*/
	0,			/*tp_as_number*/
	0,			/*tp_as_sequence*/
	&LuaObject_as_mapping,	/*tp_as_mapping*/
	0,			/*tp_hash*/
	(ternaryfunc)LuaObject_call,	      /*tp_call*/
	LuaObject_str,		 /*tp_str*/
	LuaObject_getattr,	/*tp_getattro*/
	LuaObject_setattr,	/*tp_setattro*/
	0,			/*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
	"Lua bridge object",	/*tp_doc*/
	0,			/*tp_traverse*/
	0,			/*tp_clear*/
	0,			/*tp_richcompare*/
	0,			/*tp_weaklistoffset*/
	PyObject_SelfIter,	/*tp_iter*/
	(iternextfunc)LuaObject_iternext, /*tp_iternext*/
	0,       		/*tp_methods*/
	0,       		/*tp_members*/
	0,                      /*tp_getset*/
	0,                      /*tp_base*/
	0,                      /*tp_dict*/
	0,                      /*tp_descr_get*/
	0,                      /*tp_descr_set*/
	0,                      /*tp_dictoffset*/
	0,			/*tp_init*/
	0,			/*tp_alloc*/
	0,			/*tp_new*/
	0,			/*tp_free*/
	0,                      /*tp_is_gc*/
};


PyObject *Lua_run(PyObject *args, int eval)
{
	PyObject *ret = NULL;
	char *buf = NULL;
	char *s;
	Py_ssize_t len;

	if (!PyArg_ParseTuple(args, "s#", &s, &len))
		goto error;

	if (eval) {
		size_t retlen = sizeof("return ")-1;
		size_t lenbuf = retlen + len;
		buf = PyMem_New(char, lenbuf+1);
		strcpy(buf, "return ");
		memcpy(buf+retlen, s, len);
		buf[lenbuf] = '\0';
		s = buf;
		len = lenbuf;
	}

	if (luaL_loadbuffer(L, s, len, "<python>") != 0) {
		PyErr_Format(PyExc_RuntimeError,
			     "error loading code: %s",
			     lua_tostring(L, -1));
		goto error;
	}

	PyMem_Free(buf);
	
	if (lua_pcall(L, 0, 1, 0) != 0) {
		PyErr_Format(PyExc_RuntimeError,
			     "error executing code: %s",
			     lua_tostring(L, -1));
		goto error;
	}

	ret = LuaConvert(L, -1);
  error:
	lua_settop(L, 0);
	return ret;
}

PyObject *Lua_execute(PyObject *self, PyObject *args)
{
	return Lua_run(args, 0);
}

PyObject *Lua_eval(PyObject *self, PyObject *args)
{
	return Lua_run(args, 1);
}

PyObject *Lua_globals(PyObject *self, PyObject *args)
{
	PyObject *ret = NULL;
	lua_pushvalue(L, LUA_GLOBALSINDEX);
	if (lua_isnil(L, -1)) {
		PyErr_SetString(PyExc_RuntimeError,
				"lost globals reference");
		lua_pop(L, 1);
		return NULL;
	}
	ret = LuaConvert(L, -1);
	if (!ret)
		PyErr_Format(PyExc_TypeError,
			     "failed to convert globals table");
	lua_settop(L, 0);
	return ret;
}

static PyObject *Lua_require(PyObject *self, PyObject *args)
{
	lua_pushliteral(L, "require");
	lua_rawget(L, LUA_GLOBALSINDEX);
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		PyErr_SetString(PyExc_RuntimeError, "require is not defined");
		return NULL;
	}
	return LuaCall(L, args);
}

static PyMethodDef lua_methods[] = {
	{"execute",	Lua_execute,	METH_VARARGS,		NULL},
	{"eval",	Lua_eval,	METH_VARARGS,		NULL},
	{"globals",	Lua_globals,	METH_NOARGS,		NULL},
	{"require", 	Lua_require,	METH_VARARGS,		NULL},
	{NULL,		NULL}
};

static void *py_lua_alloc(void *ud, void *ptr, size_t osize, size_t nsize, const char* name, unsigned int alloc_flags)
{
	(void)ud; (void)osize;
	if (nsize == 0) {
		PyMem_Free(ptr);
		return NULL;
	} else {
		return PyMem_Realloc(ptr, nsize);
	}
}

static int py_lua_module_panic(lua_State* L)
{
	const char *s = lua_tostring(L, -1);
	fprintf(stderr, "Error in lua module: %s\n", s);
	fprintf(stderr, "exiting (sorry)\n");
	fflush(stderr);
	return (-1);
}

#include <windows.h>

DL_EXPORT(void)
initlua(void)
{
	PyObject *m;

	LuaObject_Type.tp_new = PyType_GenericNew;
	if (PyType_Ready(&LuaObject_Type) < 0)
		return;

	m = Py_InitModule3("lua", lua_methods,
			   "Lua as a Python module.");
	if (!m)
		return;
	if (!L) {
		FILE* file;
		char filename[_MAX_PATH];
		char* slashptr;
		GetModuleFileName(GetModuleHandle("lua.pyd"), filename, _MAX_PATH);
		slashptr = strrchr(filename, '\\');
		if (slashptr) {
			slashptr++;
			strcpy(slashptr, "lua.link");
			file = fopen(filename, "r");
			if (file) {
				char buffer[ MAX_PATH ];
				if (fgets(buffer, sizeof(buffer), file) != NULL) {
					char *ptr = buffer + strlen(buffer) - 1;
					size_t len;
					while (ptr != buffer  &&  (*ptr == '\n'  ||  *ptr == '\r'))
						*ptr-- = 0;
					++ptr;
					if (*ptr != '/'  ||  *ptr != '\\') {
						*ptr++ = '/';
						*ptr = 0;
					}
					
					if (buffer[0] == '/'  ||  (buffer[1]  &&  buffer[1] == ':'))
						strcpy(filename, buffer);
					else
						strcpy(slashptr, buffer);

					len = strlen(filename);
#ifdef _DEBUG
					strcpy(filename + len, "luaplus51-1201_debug.dll");
#else
					strcpy(filename + len, "luaplus51-1201.dll");
#endif
					LoadLibrary(filename);

#ifdef _DEBUG
					strcpy(filename + len, "lua5.1.debug.dll");
#else
					strcpy(filename + len, "lua5.1.dll");
#endif
					LoadLibrary(filename);
				}
				fclose(file);
			}
		}
//		L = lua_open();
//		lua_cpcall(L, &pmain, NULL);
		L = lua_newstate(py_lua_alloc, py_lua_module_panic);
		luaL_openlibs(L);
		if (lua_cpcall(L, luaopen_python, NULL) != 0) {
			PyErr_SetString(PyExc_RuntimeError,
					"can't open python lib in lua");
		}
		lua_settop(L, 0);
	}
}
