///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2010 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUAPLUSINTERNAL_H
#define LUAPLUSINTERNAL_H

#include "src/lua.h"

NAMESPACE_LUA_BEGIN
struct lua_TValue;
NAMESPACE_LUA_END

#define LUAPLUS_ENABLE_INLINES
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

class LuaException
{
public:
	LUAPLUS_CLASS_API LuaException(const char* message);
	LUAPLUS_CLASS_API ~LuaException();
    LUAPLUS_CLASS_API LuaException(const LuaException& src);
    LUAPLUS_CLASS_API LuaException& operator=(const LuaException& src);

	const char* GetErrorMessage() const			{  return m_message;  }

protected:
	char* m_message;
};

//#define luaplus_assert(e) /* empty */
#if defined(__CELLOS_LV2__)  &&  !defined(LUAPLUS_EXCEPTIONS)
#define LUAPLUS_EXCEPTIONS 0
#endif

class LuaStateOutFile;
class LuaState;
class LuaStackObject;
class LuaObject;
class LuaCall;

struct LuaArgNil {};

} // namespace LuaPlus

#if !LUAPLUS_EXCEPTIONS
#include <assert.h>
#define luaplus_assert(e) if (!(e)) assert(0)
//(void)0
#define luaplus_throw(e) assert(0)
//(void)0
#else
#define luaplus_assert(e) if (!(e)) throw LuaPlus::LuaException(#e)
#define luaplus_throw(e) throw LuaPlus::LuaException(e)
#endif

#define LuaState_to_lua_State(state) ((lua_State*)(state))
#define lua_State_To_LuaState(L) ((LuaPlus::LuaState*)L)

#endif // LUAPLUSINTERNAL_H
