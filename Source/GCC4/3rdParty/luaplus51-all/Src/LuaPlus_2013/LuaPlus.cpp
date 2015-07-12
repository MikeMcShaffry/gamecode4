///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2005 by Joshua C. Jensen (jjensen@workspacewhiz.com).
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
#include <string.h>
#ifdef WIN32
#if defined(WIN32) && !defined(_XBOX) && !defined(_WIN32_WCE)
#include <windows.h>
#elif defined(_XBOX)
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

#include "LuaPlusFunctions.h"

#ifdef _MSC_VER
#pragma warning(disable: 4100)
#endif // _MSC_VER

//-----------------------------------------------------------------------------
LUA_EXTERN_C_BEGIN
#include "src/lualib.h"
#include "src/lstate.h"
LUA_EXTERN_C_END

// [rez] Adding some function prototypes to fix compile errors
void LuaState_UserStateOpen_Internal(lua_State* L);
void LuaState_UserStateThread_Internal(lua_State* L, lua_State* L1);
void LuaState_UserStateFree_Internal(lua_State* L);
// [rez] end

void LuaState_UserStateOpen_Internal(lua_State* L)
{
	lua_Alloc reallocFunc;
	void* data;
	lua_getdefaultallocfunction(&reallocFunc, &data);
	LuaPlus::LuaState* state = (LuaPlus::LuaState*)(*reallocFunc)(data, NULL, 0, sizeof(LuaPlus::LuaState), "LuaState", 0);
	::new(state) LuaPlus::LuaState(L);
}


void LuaState_UserStateThread_Internal(lua_State* L, lua_State* L1)
{
	lua_Alloc reallocFunc;
	void* data;
	lua_getdefaultallocfunction(&reallocFunc, &data);
	LuaPlus::LuaState* state = (LuaPlus::LuaState*)(*reallocFunc)(data, NULL, 0, sizeof(LuaPlus::LuaState), "LuaState", 0);
	::new(state) LuaPlus::LuaState(L, L1);
}


void LuaState_UserStateFree_Internal(lua_State* L)
{
	LuaPlus::LuaState* state = (LuaPlus::LuaState*)lua_getstateuserdata(L);
	if (state)
	{
		void* data;
		lua_Alloc reallocFunc = lua_getallocf(state->GetCState(), &data);
		state->~LuaState();
		(*reallocFunc)(data, state, sizeof(LuaPlus::LuaState), 0, NULL, 0);
	}
}



namespace LuaPlus {

#undef api_check
#define api_check(L, o)		(luaplus_assert(o))

#define api_checknelems(L, n)	api_check(L, (n) <= (L->top - L->ci->base))

//#define api_incr_top(L)   (api_check(L, L->top<L->ci->top), L->top++)

#define api_incr_top(L) \
	{if (L->top >= L->ci->top) lua_checkstack(L, 1); L->top++;}

LUAPLUS_API void lua_pushtobject(lua_State *L, void* tobject)
{
	TValue* tobj = (TValue*)tobject;
	lua_lock(L);
	setobj(L, L->top, tobj);
	api_incr_top(L);
	lua_unlock(L);
}

void LuaState::newthread_handler(lua_State *L, lua_State *L1)
{
	void* data;
	lua_Alloc reallocFunc = lua_getallocf(L, &data);
	LuaState* state = (LuaState*)(*reallocFunc)(data, NULL, 0, sizeof(LuaState), "CoLuaState", 0);
	::new(state) LuaState(L, L1);

//    luastateopen_thread(L);
}


void LuaState::freethread_handler(lua_State * /*L*/, lua_State *L1)
{
    LuaState::Destroy(LuaState::CastState(L1));
}


void LuaState_UserStateThread_Internal(lua_State* L, lua_State* L1);

} // namespace LuaPlus


LUA_EXTERN_C_BEGIN
#include "src/lualib.h"
#include "src/lfunc.h"
#include "src/lgc.h"
#include "src/lstate.h"
#include "src/lua.h"
#include "src/lauxlib.h"

static void *l_alloc (void *ud, void *ptr, size_t osize, size_t nsize, const char* allocName, unsigned int flags) {
  (void)osize;
  (void)ud;
  (void)allocName;
  (void)flags;
  if (nsize == 0) {
    free(ptr);
    return NULL;
  }
  else
    return realloc(ptr, nsize);
}


static lua_Alloc luaHelper_defaultAlloc = l_alloc;
static void* luaHelper_ud = NULL;

NAMESPACE_LUA_BEGIN

void lua_getdefaultallocfunction(lua_Alloc* allocFunc, void** ud)
{
	*allocFunc = luaHelper_defaultAlloc;
	*ud = luaHelper_ud;
}


void lua_setdefaultallocfunction(lua_Alloc allocFunc, void* ud)
{
	luaHelper_defaultAlloc = allocFunc ? allocFunc : l_alloc;
	luaHelper_ud = ud;
}

void reallymarkobject (global_State *g, GCObject *o);

void LuaState_UserStateOpen(lua_State* L)
{
	LuaState_UserStateOpen_Internal(L);
}


void LuaState_UserStateThread(lua_State* L, lua_State* L1)
{
	LuaState_UserStateThread_Internal(L, L1);
}


void LuaState_UserStateFree(lua_State* L)
{
	LuaState_UserStateFree_Internal(L);
}

NAMESPACE_LUA_END

LUA_EXTERN_C_END


namespace LuaPlus
{

USING_NAMESPACE_LUA


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

//static void LSLock(void* data);
//static void LSUnlock(void* data);

void LuaPlusGCFunction(void*);

/*
void LuaState_UserStateOpen_Internal(lua_State* L)
{
	lua_Alloc reallocFunc;
	void* data;
	lua_getdefaultallocfunction(&reallocFunc, &data);
	LuaState* state = (LuaState*)(*reallocFunc)(data, NULL, 0, sizeof(LuaState), "LuaState", 0);
	::new(state) LuaState(L);
}


void LuaState_UserStateThread_Internal(lua_State* L, lua_State* L1)
{
	lua_Alloc reallocFunc;
	void* data;
	lua_getdefaultallocfunction(&reallocFunc, &data);
	LuaState* state = (LuaState*)(*reallocFunc)(data, NULL, 0, sizeof(LuaState), "LuaState", 0);
	::new(state) LuaState(L, L1);
}


void LuaState_UserStateFree_Internal(lua_State* L)
{
	LuaState* state = (LuaState*)lua_getstateuserdata(L);
	if (state)
	{
		void* data;
		lua_Alloc reallocFunc = lua_getallocf(state->GetCState(), &data);
		state->~LuaState();
		(*reallocFunc)(data, state, sizeof(LuaState), 0, NULL, 0);
	}
}
*/

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4702)
#endif /* _MSC_VER */

/**
**/
static int FatalError( lua_State* state )
{
	const char* err = lua_tostring(state, 1);
#ifdef WIN32
	if (err)
		OutputDebugStringA(err);
#else // !WIN32
	if (err)
		puts(err);
#endif // WIN32

#ifndef _WIN32_WCE
	luaplus_throw(err);
#endif // _WIN32_WCE

	return -1;
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif /* _MSC_VER */


/*static*/ LuaState* LuaState::Create()
{
	lua_State* L = lua_newstate(luaHelper_defaultAlloc, luaHelper_ud);
	LuaState* state = (LuaState*)lua_getstateuserdata(L);
	state->m_ownState = true;
	lua_atpanic(L, FatalError);
	return state;
}


/*static*/ LuaState* LuaState::CastState( lua_State* L )
{
	LuaState* state = (LuaState*)lua_getstateuserdata( L );
	return state;
}


/*static*/ void LuaState::Destroy( LuaState* state )
{
	if (state->m_ownState)
		lua_close(state->m_state);
}




/**
**/
LuaState::LuaState( lua_State* state )
{
	m_state = state;

	m_ownState = false;

//jj    luaX_setnewthreadhandler( newthread_handler );
//jj    luaX_setfreethreadhandler( freethread_handler );

	lua_setstateuserdata(m_state, this);
	lua_setusergcfunction(m_state, LuaPlusGCFunction);

//    luastateopen_thread(m_state);
}


LuaState::LuaState(lua_State* L, lua_State* L1)
{
	m_state = L1;

	m_ownState = false;

	lua_setstateuserdata(m_state, this);
	lua_setusergcfunction(m_state, LuaPlusGCFunction);
}


/**
**/
LuaState::~LuaState()
{
	lua_setstateuserdata(m_state, NULL);
	m_ownState = false;
	m_state = NULL;
}


LuaObject LuaState::GetGlobals() throw()
{
	return LuaObject( this, gt(m_state) );
}


#define markvalue(g,o) { checkconsistency(o); \
  if (iscollectable(o) && iswhite(gcvalue(o))) reallymarkobject(g,gcvalue(o)); }

#define markobject(g,t) { if (iswhite(obj2gco(t))) \
		reallymarkobject(g, obj2gco(t)); }

void LuaState::LuaPlusGCFunction(void* s)
{
	lua_State* L = (lua_State*)s;
	LuaState* state = (LuaState*)lua_getstateuserdata(L);
	if (!state)
		return;

    global_State* g = G(L);

	LuaObject* curObj = (LuaObject*)G(L)->m_headObject.m_next;
	while (curObj != (LuaObject*)&G(L)->m_tailObject)
	{
		markvalue(g, curObj->GetTObject());
		curObj = (LuaObject*)(((MiniLuaObject*)curObj)->m_next);
	}
}


} // namespace LuaPlus
