///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2010 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef BUILDING_LUAPLUS
#define BUILDING_LUAPLUS
#endif
#include "LuaLink.h"
LUA_EXTERN_C_BEGIN
#include "src/lobject.h"
LUA_EXTERN_C_END
#include "LuaPlus.h"
#include "LuaState.h"
//#include "LuaCall.h"
#include "LuaPlusCD.h"

#if defined(WIN32) && !defined(_XBOX) && !defined(_XBOX_VER)
#include <windows.h>
#undef GetObject
#undef LoadString
#elif defined(_XBOX) || defined(_XBOX_VER)
#include <xtl.h>
#endif // WIN32

#ifdef _MSC_VER
#pragma warning(disable: 4702)
#endif // _MSC_VER

#include <ctype.h>

LUA_EXTERN_C_BEGIN
#include "src/lstate.h"
#include "src/lvm.h"
#include "src/lgc.h"
LUA_EXTERN_C_END


#ifndef LUAPLUS_ENABLE_INLINES
#include "LuaState.inl"
#endif // LUAPLUS_ENABLE_INLINES

namespace LuaPlus {

#if LUAPLUS_EXTENSIONS

LuaObject LuaState::GetGlobals() throw()
{
	return LuaObject( this, gt(LuaState_to_lua_State(this)) );
}

#endif // LUAPLUS_EXTENSIONS


LuaStackObject LuaState::PushVFString(const char *fmt, va_list argp) {
	lua_State* L = LuaState_to_lua_State(this);
	lua_lock(L);
	luaC_checkGC(L);
	luaO_pushvfstring(L, fmt, argp);
	lua_unlock(L);
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}


LuaStackObject LuaState::PushFString(const char *fmt, ...) {
	lua_State* L = LuaState_to_lua_State(this);
	va_list argp;
	lua_lock(L);
	luaC_checkGC(L);
	va_start(argp, fmt);
	luaO_pushvfstring(L, fmt, argp);
	va_end(argp);
	lua_unlock(L);
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}


#if LUAPLUS_EXTENSIONS

int LuaState::Equal(const LuaObject& o1, const LuaObject& o2)
{
	int i = equalobj(o1.GetCState(), o1.GetTObject(), o2.GetTObject());
	return i;
}


int LuaState::LessThan(const LuaObject& o1, const LuaObject& o2)
{
	int i = luaV_lessthan(o1.GetCState(), o1.GetTObject(), o2.GetTObject());
	return i;
}


LuaObject LuaState::NewUserDataBox(void* u)
{
	LuaObject obj(this);
	obj.AssignUserData(this, u);
	return obj;
}


LuaObject LuaState::GetLocalByName( int level, const char* name )
{
	lua_State * L = GetCState();
	lua_Debug ar;
	int i;
	const char *localName;
	if (lua_getstack(L, level, &ar) == 0)
		return LuaObject(this);  /* failure: no such level in the stack */
	i = 1;
	while ((localName = lua_getlocal(L, &ar, i++)) != NULL) {
		if (strcmp(name, localName) == 0)
		{
			LuaObject obj(this, -1);
			lua_pop(L, 1);
			return obj;
		}
		lua_pop(L, 1);  /* remove variable value */
	}
	return LuaObject(this);
}

#endif // LUAPLUS_EXTENSIONS


LuaStackObject LuaState::PushCClosure(int (*f)(LuaState*), int n)
{
	unsigned char* buffer = (unsigned char*)lua_newuserdata(LuaState_to_lua_State(this), sizeof(f));
	memcpy(buffer, &f, sizeof(f));
	Insert(-n-1);
	lua_pushcclosure(LuaState_to_lua_State(this), LPCD::LuaStateFunctionDispatcher, n + 1);
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}


#if LUAPLUS_EXTENSIONS

int LuaState::DoString( const char *str, LuaObject& fenvObj )
{
	lua_State* L = LuaState_to_lua_State(this);
	int status = luaL_loadbuffer(L, str, strlen(str), str);
	if (status != 0)
		return status;
	fenvObj.Push();
	SetFEnv(-2);
	return lua_pcall(L, 0, LUA_MULTRET, 0);
}


LuaObject LuaState::GetGlobal(const char *name)
{
	return GetGlobals()[name];
}


LuaObject LuaState::GetRegistry()
{
	return LuaObject(this, LUA_REGISTRYINDEX);  //{  lua_getregistry(LuaState_to_lua_State(this));
}


int LuaState::DoFile( const char *filename, LuaObject& fenvObj )
{
	lua_State* L = LuaState_to_lua_State(this);
	int status = luaL_loadfile(L, filename);
	if (status != 0)
		return status;
	fenvObj.Push();
	SetFEnv(-2);
	return lua_pcall(L, 0, LUA_MULTRET, 0);
}


int LuaState::DoBuffer( const char *buff, size_t size, const char *name, LuaObject& fenvObj )
{
	lua_State* L = LuaState_to_lua_State(this);
	int status = luaL_loadbuffer(L, buff, size, name);
	if (status != 0)
		return status;
	fenvObj.Push();
	SetFEnv(-2);
	return lua_pcall(L, 0, LUA_MULTRET, 0);
}


namespace LuaHelper {

LUAPLUS_API void MergeObjects(LuaObject& mergeTo, LuaObject& mergeFrom, bool replaceDuplicates)
{
	if (mergeTo.GetState() == mergeFrom.GetState())
	{
		for (LuaTableIterator it(mergeFrom); it; ++it)
		{
			LuaObject toNodeKeyObj = mergeTo[it.GetKey()];
			if (it.GetValue().IsTable())
			{
				if (toNodeKeyObj.IsNil()  ||  replaceDuplicates)
				{
					toNodeKeyObj = mergeTo.CreateTable(it.GetKey());
				}
				MergeObjects(toNodeKeyObj, it.GetValue(), replaceDuplicates);
			}
			else if (toNodeKeyObj.IsNil()  ||  replaceDuplicates)
			{
				mergeTo.SetObject(it.GetKey(), it.GetValue());
			}
		}
	}
	else
	{
		for (LuaTableIterator it(mergeFrom); it; ++it)
		{
			LuaObject obj;
			switch (it.GetKey().Type())
			{
				case LUA_TBOOLEAN:	obj.AssignBoolean(mergeTo.GetState(), it.GetKey().GetBoolean());		break;
				case LUA_TNUMBER:	obj.AssignNumber(mergeTo.GetState(), it.GetKey().GetNumber());			break;
				case LUA_TSTRING:	obj.AssignString(mergeTo.GetState(), it.GetKey().GetString());			break;
#if LUA_WIDESTRING
				case LUA_TWSTRING:	obj.AssignWString(mergeTo.GetState(), it.GetKey().GetWString());		break;
#endif /* LUA_WIDESTRING */
			}

			LuaObject toNodeKeyObj = mergeTo[obj];

			if (it.GetValue().IsTable())
			{
				if (toNodeKeyObj.IsNil()  ||  replaceDuplicates)
				{
					toNodeKeyObj = mergeTo.CreateTable(it.GetKey());
				}
				MergeObjects(toNodeKeyObj, it.GetValue(), replaceDuplicates);
			}
			else if (toNodeKeyObj.IsNil()  ||  replaceDuplicates)
			{
				LuaObject toKeyObj;
				switch (it.GetKey().Type())
				{
					case LUA_TBOOLEAN:	toKeyObj.AssignBoolean(mergeTo.GetState(), it.GetKey().GetBoolean());		break;
					case LUA_TNUMBER:	toKeyObj.AssignNumber(mergeTo.GetState(), it.GetKey().GetNumber());			break;
					case LUA_TSTRING:	toKeyObj.AssignString(mergeTo.GetState(), it.GetKey().GetString());			break;
#if LUA_WIDESTRING
					case LUA_TWSTRING:	toKeyObj.AssignWString(mergeTo.GetState(), it.GetKey().GetWString());		break;
#endif /* LUA_WIDESTRING */
				}

				switch (it.GetValue().Type())
				{
					case LUA_TBOOLEAN:	mergeTo.SetBoolean(toKeyObj, it.GetValue().GetBoolean());	break;
					case LUA_TNUMBER:	mergeTo.SetNumber(toKeyObj, it.GetValue().GetNumber());		break;
					case LUA_TSTRING:	mergeTo.SetString(toKeyObj, it.GetValue().GetString());		break;
#if LUA_WIDESTRING
					case LUA_TWSTRING:	mergeTo.SetWString(toKeyObj, it.GetValue().GetWString());	break;
#endif /* LUA_WIDESTRING */
				}
			}
		}
	}
}

} // namespace LuaHelper

#endif // LUAPLUS_EXTENSIONS

} // namespace LuaPlus


namespace LPCD
{
	void Push(lua_State* L, int (*value)(LuaState*))
	{
		LuaState* state = lua_State_To_LuaState(L);
		state->PushCClosure(value, 0);
	}
}
