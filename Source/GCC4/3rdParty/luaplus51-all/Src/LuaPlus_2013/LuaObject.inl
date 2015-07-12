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
#ifndef LUAOBJECT_INL
#define LUAOBJECT_INL

///////////////////////////////////////////////////////////////////////////////
// namespace LuaPlus
///////////////////////////////////////////////////////////////////////////////
namespace LuaPlus
{

/**
**/
inline bool LuaObject::operator==(const LuaObject& right) const
{
	luaplus_assert(m_state);
	return m_state->Equal(*this, right) != 0;
}

	
/**
**/
inline bool LuaObject::operator<(const LuaObject& right) const
{
	luaplus_assert(m_state);
	return m_state->LessThan(*this, right) != 0;
}

	
/**
**/
inline LuaState* LuaObject::GetState() const
{
	return m_state;
}


inline lua_State* LuaObject::GetCState() const
{
	return m_state->m_state;
}


inline lua_TValue* LuaObject::GetTObject() const
{
	return (lua_TValue*)&m_object;
}


} // namespace LuaPlus

#endif // LUAOBJECT_INL
