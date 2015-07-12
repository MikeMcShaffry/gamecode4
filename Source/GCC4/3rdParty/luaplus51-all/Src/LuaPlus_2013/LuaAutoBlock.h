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
#ifndef LUAAUTOBLOCK_H
#define LUAAUTOBLOCK_H

#include "LuaPlusInternal.h"

///////////////////////////////////////////////////////////////////////////////
// namespace LuaPlus
///////////////////////////////////////////////////////////////////////////////
namespace LuaPlus
{

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
/**
	A helper class for automatically setting the stack state back to point of
	LuaAutoBlock creation.
**/
class LuaAutoBlock
{
public:

	LuaAutoBlock(LuaState* state) :
		m_state(state->GetCState()),
		m_stackTop(lua_gettop(m_state))
	{
	}

	LuaAutoBlock(lua_State* state) :
		m_state(state),
		m_stackTop(lua_gettop(m_state))
	{
	}

	LuaAutoBlock(LuaStackObject& object) :
		m_state(object.GetCState()),
		m_stackTop(lua_gettop(m_state))
	{
	}

	~LuaAutoBlock()
	{
		lua_settop(m_state, m_stackTop);
	}

private:
	LuaAutoBlock(const LuaAutoBlock& src);					// Not implemented
	const LuaAutoBlock& operator=(const LuaAutoBlock& src);	// Not implemented

	lua_State* m_state;
	int m_stackTop;
};


} // namespace LuaPlus


#endif // LUAAUTOBLOCK_H
