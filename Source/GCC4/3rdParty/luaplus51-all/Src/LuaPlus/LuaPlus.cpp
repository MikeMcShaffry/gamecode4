///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2010 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef BUILDING_LUAPLUS
#define BUILDING_LUAPLUS
#endif
#include "LuaLink.h"
LUA_EXTERN_C_BEGIN
#include "src/lobject.h"
#include "src/lgc.h"
LUA_EXTERN_C_END
#include "LuaPlus.h"
#include "LuaState.h"
//#include "LuaCall.h"
#include <string.h>
#ifdef WIN32
#if defined(WIN32) && !defined(_XBOX) && !defined(_XBOX_VER) && !defined(_WIN32_WCE)
#include <windows.h>
#elif defined(_XBOX) || defined(_XBOX_VER)
#include <xtl.h>
#endif // WIN32
#undef GetObject
#endif // WIN32
#if defined(__GNUC__)
	#include <new>
#else
	#include <new.h>
#endif

#include <stdlib.h>
#include <wchar.h>
#include <assert.h>

#ifdef _MSC_VER
#pragma warning(disable: 4100)
#endif // _MSC_VER

//-----------------------------------------------------------------------------
LUA_EXTERN_C_BEGIN
#include "src/lualib.h"
#include "src/lstate.h"
#include "src/lualib.h"
#include "src/lfunc.h"
#include "src/lgc.h"
#include "src/lstate.h"
#include "src/lua.h"
#include "src/lauxlib.h"

#if LUAPLUS_EXTENSIONS
static void *luaplus_alloc (void *ud, void *ptr, size_t osize, size_t nsize, const char* allocName, unsigned int flags) {
  (void)allocName;
  (void)flags;
#else
static void *luaplus_alloc (void *ud, void *ptr, size_t osize, size_t nsize) {
#endif /* LUAPLUS_EXTENSIONS */
  (void)osize;
  (void)ud;
  if (nsize == 0) {
    free(ptr);
    return NULL;
  }
  else
    return realloc(ptr, nsize);
}


static lua_Alloc luaHelper_defaultAlloc = luaplus_alloc;
static void* luaHelper_ud = NULL;

NAMESPACE_LUA_BEGIN

void lua_getdefaultallocfunction(lua_Alloc* allocFunc, void** ud)
{
	*allocFunc = luaHelper_defaultAlloc;
	*ud = luaHelper_ud;
}


void lua_setdefaultallocfunction(lua_Alloc allocFunc, void* ud)
{
	luaHelper_defaultAlloc = allocFunc ? allocFunc : luaplus_alloc;
	luaHelper_ud = ud;
}

void reallymarkobject (global_State *g, GCObject *o);

#if LUAPLUS_EXTENSIONS
#define markvalue(g,o) { checkconsistency(o); \
  if (iscollectable(o) && iswhite(gcvalue(o))) reallymarkobject(g,gcvalue(o)); }

#define markobject(g,t) { if (iswhite(obj2gco(t))) \
		reallymarkobject(g, obj2gco(t)); }

void LuaPlusGCFunction(void* s)
{
	lua_State* L = (lua_State*)s;
	LuaPlus::LuaState* state = lua_State_To_LuaState(L);
	if (!state)
		return;

    global_State* g = G(L);

	LuaPlus::LuaObject* curObj = (LuaPlus::LuaObject*)G(L)->gchead_next;
	while (curObj != (LuaPlus::LuaObject*)&G(L)->gctail_next)
	{
		markvalue(g, curObj->GetTObject());
		curObj = *(LuaPlus::LuaObject**)curObj;
	}
}
#endif /* LUAPLUS_EXTENSIONS */

static int FatalError( lua_State* state );

void LuaState_UserStateOpen(lua_State* L)
{
#if LUAPLUS_EXTENSIONS
	lua_setusergcfunction(L, LuaPlusGCFunction);
#endif /* LUAPLUS_EXTENSIONS */
	lua_atpanic(L, FatalError);
}


NAMESPACE_LUA_END

LUA_EXTERN_C_END

namespace LuaPlus
{

USING_NAMESPACE_LUA

#if LUAPLUS_EXCEPTIONS

LuaException::LuaException(const char* message)
	: m_message(NULL)
{
	if (message)
	{
		m_message = new char[strlen(message) + 1];
		strcpy(m_message, message);
	}
}


LuaException::LuaException(const LuaException& src)
{
    m_message = new char[strlen(src.m_message) + 1];
    strcpy(m_message, src.m_message);
}


LuaException& LuaException::operator=(const LuaException& src)
{
    delete[] m_message;
    m_message = new char[strlen(src.m_message) + 1];
    strcpy(m_message, src.m_message);
    return *this;
}


LuaException::~LuaException()
{
    delete[] m_message;
}

#endif // LUAPLUS_EXCEPTIONS


/*static*/ LuaState* LuaState::Create()
{
	return lua_State_To_LuaState(lua_newstate(luaHelper_defaultAlloc, luaHelper_ud));
}


/*static*/ void LuaState::Destroy( LuaState* state )
{
	lua_State* L = LuaState_to_lua_State(state);
	if (G(L)->mainthread == L)
		lua_close(L);
}

} // namespace LuaPlus

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4702)
#endif /* _MSC_VER */

LUA_EXTERN_C_BEGIN

/**
**/
static int FatalError( lua_State* state )
{
	const char* err = lua_tostring(state, 1);
#ifdef WIN32
	if (err)
		OutputDebugString(err);
#else // !WIN32
	if (err)
		puts(err);
#endif // WIN32

#ifndef _WIN32_WCE
	luaplus_throw(err);
#endif // _WIN32_WCE

	return -1;
}

LUA_EXTERN_C_END

#if defined(_MSC_VER)
#pragma warning(pop)
#endif /* _MSC_VER */
