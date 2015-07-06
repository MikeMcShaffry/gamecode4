///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2010 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUASTACKTABLEITERATOR_INL
#define LUASTACKTABLEITERATOR_INL

///////////////////////////////////////////////////////////////////////////////
// namespace LuaPlus
///////////////////////////////////////////////////////////////////////////////
namespace LuaPlus
{

/**
	\param tableObj The table to iterate the contents of.
	\param doReset If true, the Reset() function is called at constructor
		initialization time, allowing the iterator to be used immediately.
		If false, then Reset() must be called before iterating.
	\param autoStackManagement If true, then for every Next() pass through
		the iterator, the stack is cleared to the iteration position (as if
		a LuaAutoBlock was run every single time).
**/
LUAPLUS_INLINE LuaStackTableIterator::LuaStackTableIterator(LuaStackObject& tableObj, bool doReset, bool _autoStackManagement)
	: L(tableObj.GetCState())
	, stackIndex(tableObj)
	, startStackIndex(-1)
	, autoStackManagement(_autoStackManagement) {
	// If the user requested it, perform the automatic reset.
	if (doReset)
		Reset();
}


/**
	The destructor does nothing.
**/
LUAPLUS_INLINE LuaStackTableIterator::~LuaStackTableIterator() {
}


/**
	Start iteration at the beginning of the table.
**/
LUAPLUS_INLINE void LuaStackTableIterator::Reset() {
	// Start afresh...
	startStackIndex = lua_gettop(L);

	// Push the head stack entry.
	lua_pushnil(L);

	// Start the iteration.  If the return value is 0, then the iterator
	// will be invalid.
	if (lua_next(L, stackIndex) == 0)
		startStackIndex = -1;
}


/**
	Invalidates the iterator.  Call this function if you early abort from
	an iteration loop (such as before it hits the end).
**/
LUAPLUS_INLINE void LuaStackTableIterator::Invalidate() {
	// See if the iterator is already invalid.
	if (!IsValid())
		return;

	// This is a local helper variable so we don't waste space in the class
	// definition.
	if (!autoStackManagement) {
		luaplus_assert(lua_gettop(L) <= startStackIndex + 1);
	}

	// Set the stack back.
	lua_settop(L, startStackIndex);

	// Invalidate the iterator.
	startStackIndex = -1;
}

/**
	Go to the next entry in the table.

	\return Returns true if the iteration is done.
**/
LUAPLUS_INLINE bool LuaStackTableIterator::Next() {
	// This function is only active if Reset() has been called head.
	luaplus_assert(IsValid());

	// Do any stack management operations.
	if (autoStackManagement) {
		lua_settop(L, startStackIndex + 1);
	} else {
		// If this luaplus_assert fires, then you left something on the stack.
		luaplus_assert(lua_gettop(L) == startStackIndex + 1);
	}

	// Do the Lua table iteration.
	if (lua_next(L, stackIndex) == 0 ) {
		// Invalidate the iterator.
		startStackIndex = -1;
		return false;
	}

	// The iterator is still valid.
	return true;
}


/**
	\return Returns true if the iterator is valid (there is a current element).
**/
LUAPLUS_INLINE bool LuaStackTableIterator::IsValid() const {
	return startStackIndex != -1;
}


/**
	We can easily allow a prefix operator++.  Postfix would be a stack
	management nightmare.
**/
LUAPLUS_INLINE LuaStackTableIterator& LuaStackTableIterator::operator++() {
	Next();
	return *this;
}


/**
	\return Returns true if the iterator is valid (there is a current element).
**/
LUAPLUS_INLINE LuaStackTableIterator::operator bool() const {
	// If the iterator is valid, then we're good.
	return IsValid();
}


/**
	\return Returns a LuaStackObject describing the current key.
**/
LUAPLUS_INLINE LuaStackObject LuaStackTableIterator::GetKey() {
	// This function is only active if Reset() has been called head.
	luaplus_assert(IsValid());

	return LuaStackObject(L, startStackIndex + 1);
}


/**
	\return Returns a LuaStackObject describing the current value.
**/
LUAPLUS_INLINE LuaStackObject LuaStackTableIterator::GetValue() {
	// This function is only active if Reset() has been called head.
	luaplus_assert( IsValid() );

	return LuaStackObject(L, startStackIndex + 2);
}


} // namespace LuaPlus

#endif // LUASTACKTABLEITERATOR_INL
