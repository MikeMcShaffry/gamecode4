///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2007 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://wwhiz.com/LuaPlus/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#define BUILDING_LUAPLUS
#include "LuaLink.h"
LUA_EXTERN_C_BEGIN
#define LUA_CORE
#include "src/lobject.h"
LUA_EXTERN_C_END
#include "LuaPlus.h"
#include "LuaCall.h"

//-----------------------------------------------------------------------------
namespace LuaPlus {

LuaCall::LuaCall(LuaObject& functionObj) :
	m_functionObj(functionObj),
	m_numArgs(0)
{
	luaplus_assert(m_functionObj.IsFunction());
	m_state = m_functionObj.GetState();
	m_startResults = m_state->GetTop() + 1;
	m_functionObj.Push();
}

LuaCall& operator<<(LuaCall& call, const LuaArgNil& /*value*/)
{
	call.m_state->PushNil();
	call.m_numArgs++;
	return call;
}

LuaCall& operator<<(LuaCall& call, float value)
{
	call.m_state->PushNumber(value);
	call.m_numArgs++;
	return call;
}

LuaCall& operator<<(LuaCall& call, double value)
{
	call.m_state->PushNumber(value);
	call.m_numArgs++;
	return call;
}

LuaCall& operator<<(LuaCall& call, int value)
{
	call.m_state->PushInteger(value);
	call.m_numArgs++;
	return call;
}

LuaCall& operator<<(LuaCall& call, unsigned int value)
{
	call.m_state->PushInteger(value);
	call.m_numArgs++;
	return call;
}

LuaCall& operator<<(LuaCall& call, const char* value)
{
	call.m_state->PushString(value);
	call.m_numArgs++;
	return call;
}

LuaCall& operator<<(LuaCall& call, const lua_WChar* value)
{
	call.m_state->PushWString(value);
	call.m_numArgs++;
	return call;
}

LuaCall& operator<<(LuaCall& call, lua_CFunction value)
{
	call.m_state->PushCClosure(value, 0);
	call.m_numArgs++;
	return call;
}

LuaCall& operator<<(LuaCall& call, int (*value)(LuaState*))
{
	call.m_state->PushCClosure(value, 0);
	call.m_numArgs++;
	return call;
}

LuaCall& operator<<(LuaCall& call, bool value)
{
	call.m_state->PushBoolean(value);
	call.m_numArgs++;
	return call;
}

LuaCall& operator<<(LuaCall& call, void* value)
{
	call.m_state->PushLightUserData(value);
	call.m_numArgs++;
	return call;
}

LuaCall& operator<<(LuaCall& call, LuaStackObject& value)
{
	value.Push();
	call.m_numArgs++;
	return call;
}

LuaCall& operator<<(LuaCall& call, LuaObject& value)
{
	value.Push();
	call.m_numArgs++;
	return call;
}

LuaStackObject LuaCall::operator<<(const LuaRun& run)
{
    int resultsStackPos = m_state->GetTop() - m_numArgs;
	int err = m_state->PCall(m_numArgs, run.m_numResults, run.m_alertStackPos);
	if (err != 0)
	{
		LuaStackObject errObj(m_state, -1);
        if (errObj.IsString())
        {
            // Does this string persist long enough?
            luaplus_throw(errObj.GetString());
        }
        else
        {
            char buf[200];
            sprintf(buf, "unknown lua error, code: %d", err);
            luaplus_throw(buf);
        }
	}
	return LuaStackObject(m_state, resultsStackPos);
}

LuaCall& LuaCall::operator=(const LuaCall& src)
{
	m_state = src.m_state;
	m_functionObj = src.m_functionObj;
	m_numArgs = src.m_numArgs;
	m_startResults = src.m_startResults;

	return *this;
}

} // namespace LuaPlus
