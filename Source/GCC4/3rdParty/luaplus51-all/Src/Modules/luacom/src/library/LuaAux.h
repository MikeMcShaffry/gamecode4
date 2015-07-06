// LuaAux.h: interface for the LuaAux class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LUAAUX_H__4C0212A0_1DD9_11D4_B880_0000B45D7541__INCLUDED_)
#define AFX_LUAAUX_H__4C0212A0_1DD9_11D4_B880_0000B45D7541__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

extern "C"
{
#include "lua.h"
}

typedef int stkIndex;

class LuaAux  
{
public:
	static void printLuaTable(lua_State *L, stkIndex index);
	static void printLuaStack(lua_State *L);
	static void printPreDump(int expected);
  static const char* makeLuaErrorMessage(int return_value, const char* msg);
	LuaAux();
	virtual ~LuaAux();

};

#include <assert.h>

#define LUASTACK_SET(L) const int __LuaAux_luastack_top_index = lua_gettop(L)

#ifdef NDEBUG
#define LUASTACK_CLEAN(L, n) lua_settop(L, __LuaAux_luastack_top_index + n)
#else
#define LUASTACK_CLEAN(L, n) if((__LuaAux_luastack_top_index + n) != lua_gettop(L)) { LuaAux::printPreDump(__LuaAux_luastack_top_index + n); LuaAux::printLuaStack(L); assert(0); }
#endif

#endif // !defined(AFX_LUAAUX_H__4C0212A0_1DD9_11D4_B880_0000B45D7541__INCLUDED_)
