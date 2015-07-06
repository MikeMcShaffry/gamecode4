///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2010 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUACALL_H
#define LUACALL_H

#include "LuaPlusInternal.h"
#include "LuaObject.h"

namespace LuaPlus {

struct LuaRun
{
    LuaRun(int _numResults = -1, int _alertStackPos = 0) throw() :
        numResults(_numResults),
        alertStackPos(_alertStackPos) {
	}

	int numResults;
    int alertStackPos;
};


class LuaCall {
public:
	LuaCall(LuaObject& functionObj);

	LuaStackObject operator<<(const LuaRun& /*run*/);
	LuaCall& operator=(const LuaCall& src);

	lua_State* L;
	int numArgs;
	int startResults;
};


LUAPLUS_INLINE LuaCall& operator<<(LuaCall& call, const LuaArgNil& value);
LUAPLUS_INLINE LuaCall& operator<<(LuaCall& call, float value);
LUAPLUS_INLINE LuaCall& operator<<(LuaCall& call, double value);
LUAPLUS_INLINE LuaCall& operator<<(LuaCall& call, int value);
LUAPLUS_INLINE LuaCall& operator<<(LuaCall& call, unsigned int value);
LUAPLUS_INLINE LuaCall& operator<<(LuaCall& call, const char* value);
#if LUA_WIDESTRING
LUAPLUS_INLINE LuaCall& operator<<(LuaCall& call, const lua_WChar* value);
#endif /* LUA_WIDESTRING */
LUAPLUS_INLINE LuaCall& operator<<(LuaCall& call, lua_CFunction value);
LUAPLUS_INLINE LuaCall& operator<<(LuaCall& call, int (*value)(LuaState*));
LUAPLUS_INLINE LuaCall& operator<<(LuaCall& call, bool value);
LUAPLUS_INLINE LuaCall& operator<<(LuaCall& call, void* value);
LUAPLUS_INLINE LuaCall& operator<<(LuaCall& call, LuaStackObject& value);
LUAPLUS_INLINE LuaCall& operator<<(LuaCall& call, LuaObject& value);

} // namespace LuaPlus

#ifdef LUAPLUS_ENABLE_INLINES
#include "LuaCall.inl"
#endif // LUAPLUS_ENABLE_INLINES

#endif // LUACALL_H

