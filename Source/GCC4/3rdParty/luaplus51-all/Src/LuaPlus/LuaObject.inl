///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2010 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUAOBJECT_INL
#define LUAOBJECT_INL

#if LUAPLUS_EXTENSIONS

///////////////////////////////////////////////////////////////////////////////
// namespace LuaPlus
///////////////////////////////////////////////////////////////////////////////
namespace LuaPlus
{

/**
**/
inline bool LuaObject::operator==(const LuaObject& right) const
{
	luaplus_assert(L);
	return lua_State_To_LuaState(L)->Equal(*this, right) != 0;
}

	
/**
**/
inline bool LuaObject::operator<(const LuaObject& right) const
{
	luaplus_assert(L);
	return lua_State_To_LuaState(L)->LessThan(*this, right) != 0;
}

	
/**
**/
inline LuaState* LuaObject::GetState() const
{
	return lua_State_To_LuaState(L);
}


inline lua_State* LuaObject::GetCState() const
{
	return L;
}


inline lua_TValue* LuaObject::GetTObject() const
{
	return (lua_TValue*)&m_object;
}


/**
**/
inline void LuaObject::AssignCFunction(NAMESPACE_LUA_PREFIX lua_CFunction function, int nupvalues)
{
	AssignCFunctionHelper(function, nupvalues, NULL, 0, NULL, 0);
}


/**
**/
inline void LuaObject::AssignCFunction(int (*func)(LuaState*), int nupvalues)
{
	AssignCFunctionHelper(LPCD::LuaStateFunctionDispatcher, nupvalues, NULL, 0, &func, sizeof(func));
}


} // namespace LuaPlus

#endif // LUAPLUS_EXTENSIONS

#endif // LUAOBJECT_INL
