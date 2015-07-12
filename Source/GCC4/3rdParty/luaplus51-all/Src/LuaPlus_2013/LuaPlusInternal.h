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
#ifndef LUAPLUSINTERNAL_H
#define LUAPLUSINTERNAL_H

#ifdef _MSC_VER
#pragma warning(disable: 4505)
#pragma warning(disable: 4127)
#endif // _MSC_VER

#include "src/lua.h"

NAMESPACE_LUA_BEGIN
struct lua_TValue;
NAMESPACE_LUA_END

#ifdef LUAPLUS_ENABLE_INLINES
#define LUAPLUS_INLINE inline
#else // !LUAPLUS_ENABLE_INLINES
#define LUAPLUS_INLINE
#endif // LUAPLUS_ENABLE_INLINES

///////////////////////////////////////////////////////////////////////////////
// namespace LuaPlus
///////////////////////////////////////////////////////////////////////////////
namespace LuaPlus
{

USING_NAMESPACE_LUA;

class LUAPLUS_CLASS LuaException
{
public:
	LuaException(const char* message);
	~LuaException();
    LuaException(const LuaException& src);
    LuaException& operator=(const LuaException& src);

	const char* GetErrorMessage() const			{  return m_message;  }

protected:
	char* m_message;
};

//#define luaplus_assert(e) /* empty */
#ifndef LUAPLUS_EXCEPTIONS
#define LUAPLUS_EXCEPTIONS 0
#endif // LUAPLUS_EXCEPTIONS

#if !LUAPLUS_EXCEPTIONS
#include <assert.h>
#define luaplus_assert(e) if (!(e)) assert(0)
//(void)0
#define luaplus_throw(e) assert(0)
//(void)0
#else
#define luaplus_assert(e) if (!(e)) throw LuaException(#e)
#define luaplus_throw(e) throw LuaException(e)
#endif

class LuaStateOutFile;
class LuaState;
class LuaStackObject;
class LuaObject;
class LuaCall;

struct LuaArgNil {};

} // namespace LuaPlus

#endif // LUAPLUSINTERNAL_H
