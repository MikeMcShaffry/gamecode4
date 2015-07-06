///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2010 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUAAUTOBLOCK_H
#define LUAAUTOBLOCK_H

#include "LuaPlusInternal.h"

///////////////////////////////////////////////////////////////////////////////
// namespace LuaPlus
///////////////////////////////////////////////////////////////////////////////
namespace LuaPlus {

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
/**
	A helper class for automatically setting the stack state back to point of
	LuaAutoBlock creation.
**/
class LuaAutoBlock {
public:
	LuaAutoBlock(LuaState* state)
		: L(LuaState_to_lua_State(state))
		, stackTop(lua_gettop(L)) {
	}

	LuaAutoBlock(lua_State* _L)
		: L(_L)
		, stackTop(lua_gettop(L)) {
	}

	LuaAutoBlock(LuaStackObject& object)
		: L(object.GetCState())
		, stackTop(lua_gettop(L)) {
	}

	~LuaAutoBlock() {
		lua_settop(L, stackTop);
	}

private:
	LuaAutoBlock(const LuaAutoBlock& src);					// Not implemented
	const LuaAutoBlock& operator=(const LuaAutoBlock& src);	// Not implemented

	lua_State* L;
	int stackTop;
};

} // namespace LuaPlus

#endif // LUAAUTOBLOCK_H
