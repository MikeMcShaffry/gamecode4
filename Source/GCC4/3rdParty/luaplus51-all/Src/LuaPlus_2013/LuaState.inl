///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2005 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://wwhiz.com/LuaPlus/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER
#ifndef LUASTATE_INL
#define LUASTATE_INL

#include <string.h>
#include <assert.h>

///////////////////////////////////////////////////////////////////////////////
// namespace LuaPlus
///////////////////////////////////////////////////////////////////////////////
namespace LuaPlus
{

LUAPLUS_INLINE lua_CFunction LuaState::AtPanic(lua_CFunction panicf)
{
	return lua_atpanic(m_state, panicf);
}

// Basic stack manipulation.
LUAPLUS_INLINE int LuaState::GetTop()
{
	return lua_gettop(m_state);
}

LUAPLUS_INLINE void LuaState::SetTop(int index)
{
	lua_settop(m_state, index);
}

LUAPLUS_INLINE void LuaState::PushValue(int index)
{
	lua_pushvalue(m_state, index);
}

LUAPLUS_INLINE void LuaState::PushValue(LuaStackObject& object)
{
	lua_pushvalue(m_state, object);
}

LUAPLUS_INLINE void LuaState::Remove(int index)
{
	lua_remove(m_state, index);
}

LUAPLUS_INLINE void LuaState::Insert(int index)
{
	lua_insert(m_state, index);
}

LUAPLUS_INLINE void LuaState::Replace(int index)
{
	lua_replace(m_state, index);
}


LUAPLUS_INLINE int LuaState::CheckStack(int size)
{
	return lua_checkstack(m_state, size);
}


LUAPLUS_INLINE void LuaState::XMove(LuaState* to, int n)
{
	lua_xmove(m_state, to->m_state, n);
}

	
// access functions (stack -> C)
LUAPLUS_INLINE int LuaState::Equal(int index1, int index2)
{
	return lua_equal(m_state, index1, index2);
}

LUAPLUS_INLINE int LuaState::RawEqual(int index1, int index2)
{
	return lua_rawequal(m_state, index1, index2);
}

LUAPLUS_INLINE int LuaState::LessThan(int index1, int index2)
{
	return lua_lessthan(m_state, index1, index2);
}


// push functions (C -> stack)
LUAPLUS_INLINE LuaStackObject LuaState::PushNil()
{
	lua_pushnil(m_state);
	return LuaStackObject(this, lua_gettop(m_state));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushNumber(lua_Number n)
{
	lua_pushnumber(m_state, n);
	return LuaStackObject(this, lua_gettop(m_state));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushInteger(int n)
{
	lua_pushnumber(m_state, n);
	return LuaStackObject(this, lua_gettop(m_state));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushLString(const char *s, size_t len)
{
	lua_pushlstring(m_state, s, len);
	return LuaStackObject(this, lua_gettop(m_state));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushLWString(const lua_WChar* s, size_t len)
{
	lua_pushlwstring(m_state, s, len);
	return LuaStackObject(this, lua_gettop(m_state));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushString(const char *s)
{
	lua_pushstring(m_state, s);
	return LuaStackObject(this, lua_gettop(m_state));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushWString(const lua_WChar* s)
{
	lua_pushwstring(m_state, s);
	return LuaStackObject(this, lua_gettop(m_state));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushCClosure(lua_CFunction fn, int n)
{
	lua_pushcclosure(m_state, fn, n);
	return LuaStackObject(this, lua_gettop(m_state));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushCFunction(lua_CFunction f)
{
	lua_pushcclosure(m_state, f, 0);
	return LuaStackObject(this, lua_gettop(m_state));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushBoolean(bool value)
{
	lua_pushboolean(m_state, value);
	return LuaStackObject(this, lua_gettop(m_state));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushLightUserData(void* p)
{
	lua_pushlightuserdata(m_state, p);
	return LuaStackObject(this, lua_gettop(m_state));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushThread()
{
	lua_pushthread(m_state);
	return LuaStackObject(this, lua_gettop(m_state));
}


LUAPLUS_INLINE LuaStackObject LuaState::CreateTable(int narr, int nrec)
{
	lua_createtable(m_state, narr, nrec);
	return LuaStackObject(*this, GetTop());
}


LUAPLUS_INLINE LuaStackObject LuaState::NewUserData(size_t size)
{
	lua_newuserdata(m_state, size);
	return LuaStackObject(*this, GetTop());
}

// get functions (Lua -> stack)
LUAPLUS_INLINE void LuaState::GetTable(int index)
{
	lua_gettable(m_state, index);
}

LUAPLUS_INLINE void LuaState::RawGet(int index)
{
	lua_rawget(m_state, index);
}

LUAPLUS_INLINE void LuaState::RawGetI(int index, int n)
{
	lua_rawgeti(m_state, index, n);
}

LUAPLUS_INLINE LuaStackObject LuaState::GetMetaTable(int index)
{
	lua_getmetatable(m_state, index);  return LuaStackObject(this, lua_gettop(m_state));
}

LUAPLUS_INLINE LuaStackObject LuaState::GetDefaultMetaTable(int type)
{
	lua_getdefaultmetatable(m_state, type);  return LuaStackObject(this, lua_gettop(m_state));
}

LUAPLUS_INLINE LuaStackObject LuaState::GetGlobals_Stack()
{
	return LuaStackObject(*this, LUA_GLOBALSINDEX);
}

LUAPLUS_INLINE LuaStackObject LuaState::GetGlobal_Stack(const char *name)
{
	lua_getglobal(m_state, name);  return LuaStackObject(*this, GetTop());
}

LUAPLUS_INLINE LuaStackObject LuaState::GetRegistry_Stack()
{
	return LuaStackObject(*this, LUA_REGISTRYINDEX);  //{  lua_getregistry(m_state);
}


// set functions(stack -> Lua)
LUAPLUS_INLINE void LuaState::SetTable(int index)
{
	lua_settable(m_state, index);
}

LUAPLUS_INLINE void LuaState::RawSet(int index)
{
	lua_rawset(m_state, index);
}

LUAPLUS_INLINE void LuaState::RawSetI(int index, int n)
{
	lua_rawseti(m_state, index, n);
}

LUAPLUS_INLINE void LuaState::SetMetaTable(int index)
{
	lua_setmetatable(m_state, index);
}

LUAPLUS_INLINE void LuaState::SetDefaultMetaTable(int type)
{
	lua_setdefaultmetatable(m_state, type);
}

LUAPLUS_INLINE void LuaState::SetFEnv(int index)
{
	lua_setfenv(m_state, index);
}

LUAPLUS_INLINE int LuaState::Ref(int t)
{
	return luaL_ref(m_state, t);
}

LUAPLUS_INLINE void LuaState::Unref(int t, int ref)
{
	luaL_unref(m_state, t, ref);
}


// `load' and `do' functions (load and run Lua code)
LUAPLUS_INLINE void LuaState::Call(int nargs, int nresults)
{
	lua_call(m_state, nargs, nresults);
}

LUAPLUS_INLINE int LuaState::PCall(int nargs, int nresults, int errf)
{
	return lua_pcall(m_state, nargs, nresults, errf);
}

LUAPLUS_INLINE int LuaState::CPCall(lua_CFunction func, void* ud)
{
	return lua_cpcall(m_state, func, ud);
}

LUAPLUS_INLINE int LuaState::Load(lua_Reader reader, void *dt, const char *chunkname)
{
	return lua_load(m_state, reader, dt, chunkname);
}

LUAPLUS_INLINE int LuaState::WLoad(lua_Reader reader, void *dt, const char *chunkname)
{
	return lua_wload(m_state, reader, dt, chunkname);
}

LUAPLUS_INLINE int LuaState::Dump(lua_Chunkwriter writer, void* data, int strip, char endian)
{
	return lua_dump(m_state, writer, data, strip, endian);
}

LUAPLUS_INLINE int LuaState::LoadFile(const char* filename)
{
	return luaL_loadfile(m_state, filename);
}

LUAPLUS_INLINE int LuaState::DoFile(const char *filename)
{
	return luaL_dofile(m_state, filename);
}

LUAPLUS_INLINE void callalert (lua_State *L, int status) {
  if (status != 0) {
    lua_getglobal(L, "_ALERT");
    if (lua_isfunction(L, -1)) {
      lua_insert(L, -2);
      lua_call(L, 1, 0);
    }
    else {  /* no _ALERT function; print it on stderr */
      fprintf(stderr, "%s\n", lua_tostring(L, -2));
      lua_pop(L, 2);  /* remove error message and _ALERT */
    }
  }
}

#if 0
static int traceback (lua_State *L)
{
	luaL_getfield(L, LUA_GLOBALSINDEX, "debug.traceback");
	if (!lua_isfunction(L, -1))
		lua_pop(L, 1);
	else
	{
		lua_pushvalue(L, 1);  /* pass error message */
		lua_pushinteger(L, 2);  /* skip this function and traceback */
		lua_call(L, 2, 1);  /* call debug.traceback */
	}
	return 1;
}
#endif


LUAPLUS_INLINE int aux_do (lua_State *L, int status) {
  if (status == 0) {  /* parse OK? */
    status = lua_pcall(L, 0, LUA_MULTRET, 0);  /* call main */
  }
  callalert(L, status);
  return status;
}


LUAPLUS_INLINE int LuaState::DoString(const char *str)
{
	return luaL_dostring(m_state, str);
}

LUAPLUS_INLINE int LuaState::DoWString(const lua_WChar *str, const char* name)
{
	(void)name;
	return luaL_dowstring(m_state, str);
}

LUAPLUS_INLINE int LuaState::LoadBuffer(const char* buff, size_t size, const char* name)
{
	return luaL_loadbuffer(m_state, buff, size, name);
}

LUAPLUS_INLINE int LuaState::DoBuffer(const char *buff, size_t size, const char *name)
{
	return luaL_dobuffer(m_state, buff, size, name);
}

LUAPLUS_INLINE int LuaState::LoadWBuffer(const lua_WChar* buff, size_t size, const char* name)
{
	return luaL_loadwbuffer(m_state, buff, size, name);
}

LUAPLUS_INLINE int LuaState::DoWBuffer(const lua_WChar* buff, size_t size, const char *name)
{
	return luaL_dowbuffer(m_state, buff, size, name);

}

// coroutine functions
LUAPLUS_INLINE int LuaState::CoYield(int nresults)
{
	return lua_yield(m_state, nresults);
}

LUAPLUS_INLINE int LuaState::CoResume(int narg)
{
	return lua_resume(m_state, narg);
}

LUAPLUS_INLINE int LuaState::CoStatus()
{
	return lua_status(m_state);
}

// Miscellaneous functions
LUAPLUS_INLINE int LuaState::Error()
{
	return lua_error(m_state);
}


LUAPLUS_INLINE int LuaState::Next(int index)
{
	return lua_next(m_state, index);
}

LUAPLUS_INLINE void LuaState::Concat(int n)
{
	lua_concat(m_state, n);
}


LUAPLUS_INLINE lua_Alloc LuaState::GetAllocF(void **ud)
{
	return lua_getallocf(m_state, ud);
}


LUAPLUS_INLINE void LuaState::SetAllocF(lua_Alloc f, void *ud)
{
	lua_setallocf(m_state, f, ud);
}

	
LUAPLUS_INLINE LuaStackObject LuaState::NewUserDataBox_Stack(void* u)
{
	lua_newuserdatabox(m_state, u);  return LuaStackObject(*this, lua_gettop(m_state));
}


// Helper functions
LUAPLUS_INLINE void LuaState::Pop()
{
	lua_pop(m_state, 1);
}

LUAPLUS_INLINE void LuaState::Pop(int amount)
{
	lua_pop(m_state, amount);
}


LUAPLUS_INLINE int LuaState::GC(int what, int data)
{
	return lua_gc(m_state, what, data); 
}


// Debug functions.
LUAPLUS_INLINE int LuaState::GetStack(int level, lua_Debug* ar)
{
	return lua_getstack(m_state, level, ar);
}

LUAPLUS_INLINE int LuaState::GetInfo(const char* what, lua_Debug* ar)
{
	return lua_getinfo(m_state, what, ar);
}

LUAPLUS_INLINE const char* LuaState::GetLocal(const lua_Debug* ar, int n)
{
	return lua_getlocal(m_state, ar, n);
}

LUAPLUS_INLINE const char* LuaState::SetLocal(const lua_Debug* ar, int n)
{
	return lua_setlocal(m_state, ar, n);
}


LUAPLUS_INLINE int LuaState::SetHook(lua_Hook func, int mask, int count)
{
	return lua_sethook(m_state, func, mask, count);
}

LUAPLUS_INLINE lua_Hook LuaState::GetHook()
{
	return lua_gethook(m_state);
}

LUAPLUS_INLINE int LuaState::GetHookMask()
{
	return lua_gethookmask(m_state);
}


// Extra
LUAPLUS_INLINE LuaStackObject LuaState::BoxPointer(void* u)
{
	lua_boxpointer(m_state, u);  return LuaStackObject(*this, lua_gettop(m_state));
}

LUAPLUS_INLINE void* LuaState::UnBoxPointer(int stackIndex)
{
	return lua_unboxpointer(m_state, stackIndex);
}



} // namespace LuaPlus

#endif // LUASTATE_INL
