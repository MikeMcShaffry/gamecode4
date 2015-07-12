///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2004 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://wwhiz.com/LuaPlus/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER
#ifndef LUACALL_H
#define LUACALL_H

#include "LuaPlusInternal.h"
#include "LuaObject.h"

namespace LuaPlus {

struct LuaRun
{
    LuaRun(int numResults = -1, int alertStackPos = 0) throw() :
        m_numResults(numResults),
        m_alertStackPos(alertStackPos)
	{
	}

	int m_numResults;
    int m_alertStackPos;
};


class LUAPLUS_CLASS LuaCall
{
public:
	LuaCall(LuaObject& functionObj);

	LuaStackObject operator<<(const LuaRun& /*run*/);
	LuaCall& operator=(const LuaCall& src);

	LuaObject m_functionObj;
	LuaState* m_state;
	int m_numArgs;
	int m_startResults;
};


LUAPLUS_API LuaCall& operator<<(LuaCall& call, const LuaArgNil& value);
LUAPLUS_API LuaCall& operator<<(LuaCall& call, float value);
LUAPLUS_API LuaCall& operator<<(LuaCall& call, double value);
LUAPLUS_API LuaCall& operator<<(LuaCall& call, int value);
LUAPLUS_API LuaCall& operator<<(LuaCall& call, unsigned int value);
LUAPLUS_API LuaCall& operator<<(LuaCall& call, const char* value);
LUAPLUS_API LuaCall& operator<<(LuaCall& call, const lua_WChar* value);
LUAPLUS_API LuaCall& operator<<(LuaCall& call, lua_CFunction value);
LUAPLUS_API LuaCall& operator<<(LuaCall& call, int (*value)(LuaState*));
LUAPLUS_API LuaCall& operator<<(LuaCall& call, bool value);
LUAPLUS_API LuaCall& operator<<(LuaCall& call, void* value);
LUAPLUS_API LuaCall& operator<<(LuaCall& call, LuaStackObject& value);
LUAPLUS_API LuaCall& operator<<(LuaCall& call, LuaObject& value);

} // namespace LuaPlus

#endif // LUACALL_H

