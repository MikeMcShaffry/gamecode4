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
LUA_EXTERN_C_END
#include "LuaPlus.h"
#include "LuaCall.h"
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

#if LUAPLUS_EXTENSIONS

//-----------------------------------------------------------------------------
LUA_EXTERN_C_BEGIN
#include "src/ltable.h"
#include "src/lstate.h"

NAMESPACE_LUA_BEGIN
LUA_EXTERN_C int luaH_findindex (lua_State *L, Table *t, StkId key);
NAMESPACE_LUA_END
LUA_EXTERN_C_END

namespace LuaPlus {

LUAPLUS_API bool LuaPlusH_next(LuaState* state, LuaObject* table, LuaObject* key, LuaObject* value)
{
	Table* t = hvalue(table->GetTObject());
	int i = luaH_findindex(*state, t, key->GetTObject());  /* find original element */
    for (i++; i < t->sizearray; i++) {  /* try first array part */
      if (!ttisnil(&t->array[i])) {  /* a non-nil value? */
        key->AssignInteger(state, i + 1);
        value->AssignTObject(state, &t->array[i]);
        return true;
	  }
	}
    for (i -= t->sizearray; i < sizenode(t); i++) {  /* then hash part */
      if (!ttisnil(gval(gnode(t, i)))) {  /* a non-nil value? */
        key->AssignTObject(state, key2tval(gnode(t, i)));
        value->AssignTObject(state, gval(gnode(t, i)));
        return true;
      }
	}
	return false;  /* no more elements */
}



/**
	\param tableObj The table to iterate the contents of.
	\param doReset If true, the Reset() function is called at constructor
		initialization time, allowing the iterator to be used immediately.
		If false, then Reset() must be called before iterating.
**/
LuaTableIterator::LuaTableIterator( const LuaObject& tableObj, bool doReset ) :
	m_keyObj(tableObj.GetState()),
	m_valueObj(tableObj.GetState()),
	m_tableObj(tableObj),
	m_isDone(false) {
	luaplus_assert(tableObj.IsTable());

	// If the user requested it, perform the automatic reset.
	if ( doReset )
		Reset();
}


/**
	The destructor.
**/
LuaTableIterator::~LuaTableIterator() {
}


/**
	Start iteration at the beginning of the table.
**/
void LuaTableIterator::Reset() {
	// Start afresh...
	LuaState* state = m_tableObj.GetState();

	// Start at the beginning.
	m_keyObj.AssignNil(state);

	// Start the iteration.  If the return value is 0, then the iterator
	// will be invalid.
	m_isDone = !LuaPlusH_next(state, &m_tableObj, &m_keyObj, &m_valueObj);
}


/**
	Invalidates the iterator.  Call this function if you early abort from
	an iteration loop (such as before it hits the end).
**/
void LuaTableIterator::Invalidate() {
	// This is a local helper variable so we don't waste space in the class
	// definition.
	LuaState* state = m_tableObj.GetState();
	m_keyObj.AssignNil(state);
	m_valueObj.AssignNil(state);
}

/**
	Go to the next entry in the table.

	\return Returns true if the iteration is done.
**/
bool LuaTableIterator::Next() {
	// This function is only active if Reset() has been called head.
	luaplus_assert( IsValid() );

	// This is a local helper variable so we don't waste space in the class
	// definition.
	LuaState* state = m_tableObj.GetState();

	// Do the Lua table iteration.
	m_isDone = !LuaPlusH_next(state, &m_tableObj, &m_keyObj, &m_valueObj);
	return !m_isDone;
}


/**
	\return Returns true if the iterator is valid (there is a current element).
**/
bool LuaTableIterator::IsValid() const {
	return !m_isDone;
}


/**
	We can easily allow a prefix operator++.  Postfix would be a stack
	management nightmare.
**/
LuaTableIterator& LuaTableIterator::operator++() {
	Next();
	return *this;
}


/**
	\return Returns true if the iterator is valid (there is a current element).
**/
LuaTableIterator::operator bool() const {
	// If the iterator is valid, then we're good.
	return IsValid();
}


/**
	\return Returns a LuaObject describing the current key.
**/
LuaObject& LuaTableIterator::GetKey() {
	// This function is only active if Reset() has been called head.
	luaplus_assert( IsValid() );

	return m_keyObj;
}


/**
	\return Returns a LuaObject describing the current value.
**/
LuaObject& LuaTableIterator::GetValue() {
	// This function is only active if Reset() has been called head.
	luaplus_assert( IsValid() );

	return m_valueObj;
}

} // namespace LuaPlus

#endif // LUAPLUS_EXTENSIONS

