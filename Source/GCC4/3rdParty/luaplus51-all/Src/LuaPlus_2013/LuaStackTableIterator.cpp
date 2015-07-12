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

namespace LuaPlus {

/**
	\param tableObj The table to iterate the contents of.
	\param doReset If true, the Reset() function is called at constructor
		initialization time, allowing the iterator to be used immediately.
		If false, then Reset() must be called before iterating.
	\param autoStackManagement If true, then for every Next() pass through
		the iterator, the stack is cleared to the iteration position (as if
		a LuaAutoBlock was run every single time).
**/
LuaStackTableIterator::LuaStackTableIterator( LuaStackObject tableObj, bool doReset,
		bool autoStackManagement ) :
	m_tableObj( tableObj ),
	m_startStackIndex( -1 ),
	m_autoStackManagement( autoStackManagement )
{
	// If the user requested it, perform the automatic reset.
	if ( doReset )
		Reset();
}


/**
	The destructor does nothing.
**/
LuaStackTableIterator::~LuaStackTableIterator()
{
};


/**
	Start iteration at the beginning of the table.
**/
void LuaStackTableIterator::Reset()
{
	// Start afresh...
	LuaState* state = m_tableObj.GetState();
	m_startStackIndex = state->GetTop();

	// Push the head stack entry.
	state->PushNil();

	// Start the iteration.  If the return value is 0, then the iterator
	// will be invalid.
	if ( state->Next( m_tableObj ) == 0 )
		m_startStackIndex = -1;
}


/**
	Invalidates the iterator.  Call this function if you early abort from
	an iteration loop (such as before it hits the end).
**/
void LuaStackTableIterator::Invalidate()
{
	// See if the iterator is already invalid.
	if ( !IsValid() )
		return;

	// This is a local helper variable so we don't waste space in the class
	// definition.
	LuaState* state = m_tableObj.GetState();

	if ( !m_autoStackManagement )
	{
		luaplus_assert( state->GetTop() <= m_startStackIndex + 1 );
	}

	// Set the stack back.
	state->SetTop( m_startStackIndex );

	// Invalidate the iterator.
	m_startStackIndex = -1;
}

/**
	Go to the next entry in the table.

	\return Returns true if the iteration is done.
**/
bool LuaStackTableIterator::Next()
{
	// This function is only active if Reset() has been called head.
	luaplus_assert( IsValid() );

	// This is a local helper variable so we don't waste space in the class
	// definition.
	LuaState* state = m_tableObj.GetState();

	// Do any stack management operations.
	if ( m_autoStackManagement )
	{
		state->SetTop( m_startStackIndex + 1 );
	}
	else
	{
		// If this luaplus_assert fires, then you left something on the stack.
		luaplus_assert( state->GetTop() == m_startStackIndex + 1 );
	}

	// Do the Lua table iteration.
	if ( state->Next( m_tableObj ) == 0 )
	{
		// Invalidate the iterator.
		m_startStackIndex = -1;
		return false;
	}

	// The iterator is still valid.
	return true;
}


/**
	\return Returns true if the iterator is valid (there is a current element).
**/
bool LuaStackTableIterator::IsValid() const
{
	return m_startStackIndex != -1;
}


/**
	We can easily allow a prefix operator++.  Postfix would be a stack
	management nightmare.
**/
LuaStackTableIterator& LuaStackTableIterator::operator++()
{
	Next();
	return *this;
}


/**
	\return Returns true if the iterator is valid (there is a current element).
**/
LuaStackTableIterator::operator bool() const
{
	// If the iterator is valid, then we're good.
	return IsValid();
}


/**
	\return Returns a LuaStackObject describing the current key.
o**/
LuaStackObject LuaStackTableIterator::GetKey()
{
	// This function is only active if Reset() has been called head.
	luaplus_assert( IsValid() );

	return LuaStackObject( m_tableObj.GetState(), m_startStackIndex + 1 );
}


/**
	\return Returns a LuaStackObject describing the current value.
**/
LuaStackObject LuaStackTableIterator::GetValue()
{
	// This function is only active if Reset() has been called head.
	luaplus_assert( IsValid() );

	return LuaStackObject( m_tableObj.GetState(), m_startStackIndex + 2 );
}


} // namespace LuaPlus
