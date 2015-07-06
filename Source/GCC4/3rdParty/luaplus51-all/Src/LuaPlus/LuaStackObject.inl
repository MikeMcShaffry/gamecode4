///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2010 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUASTACKOBJECT_INL
#define LUASTACKOBJECT_INL

///////////////////////////////////////////////////////////////////////////////
// namespace LuaPlus
///////////////////////////////////////////////////////////////////////////////
namespace LuaPlus
{

/**
	Constructor.
**/
LUAPLUS_INLINE LuaStackObject::LuaStackObject( lua_State* _L, int stackIndex ) :
	L( _L ),
	m_stackIndex( stackIndex )
{
}


/**
	Constructor.
**/
LUAPLUS_INLINE LuaStackObject::LuaStackObject( LuaState* state, int stackIndex ) :
	L( LuaState_to_lua_State( state ) ),
	m_stackIndex( stackIndex )
{
}


/**
**/
LUAPLUS_INLINE bool LuaStackObject::operator==(const LuaStackObject& right) const
{
	return lua_equal(LuaState_to_lua_State(L), (int)*this, (int)right) != 0;
}

	
/**
**/
LUAPLUS_INLINE LuaState* LuaStackObject::GetState() const
{
	return lua_State_To_LuaState( L );
}


LUAPLUS_INLINE lua_State* LuaStackObject::GetCState() const			{  return L;  }

/**
	Retrieves the type name.  Corresponds to the information presented in section
	4.5 of the Lua manual.
**/
LUAPLUS_INLINE const char* LuaStackObject::GetTypeName() const
{
	return lua_typename( GetCState(), m_stackIndex );
}


/**
	Returns one of the LUA_T* constants, based on the type.  Corresponds to section
	4.5 of the Lua manual.

	\return Returns one of the following constants: LUA_TNIL, LUA_TINTEGER, LUA_TNUMBER,
		LUA_TSTRING, LUA_TBOOLEAN, LUA_TTABLE, LUA_TFUNCTION, LUA_TUSERDATA,
		LUA_TLIGHTUSERDATA, LUA_TWSTRING
**/
LUAPLUS_INLINE int LuaStackObject::GetType() const
{
	return lua_type( GetCState(), m_stackIndex );
}


/**
	\return Returns true if the object is nil.
**/
LUAPLUS_INLINE bool LuaStackObject::IsNil() const					{  return lua_isnil( GetCState(), m_stackIndex );  }


/**
	\return Returns true if the object is a table.
**/
LUAPLUS_INLINE bool LuaStackObject::IsTable() const					{  return lua_istable( GetCState(), m_stackIndex );  }


/**
	\return Returns true if the object is user data, including light user data.
**/
LUAPLUS_INLINE bool LuaStackObject::IsUserData() const				{  return lua_isuserdata( GetCState(), m_stackIndex ) != 0;  }


/**
	\return Returns true if the object is a C function callback.
**/
LUAPLUS_INLINE bool LuaStackObject::IsCFunction() const				{  return lua_iscfunction( GetCState(), m_stackIndex ) != 0;  }


/**
	\return Returns true if the object is a number.
**/
LUAPLUS_INLINE bool LuaStackObject::IsInteger() const					{  return lua_type( GetCState(), m_stackIndex ) == LUA_TNUMBER;  }


/**
	\return Returns true if the object is a number.
**/
LUAPLUS_INLINE bool LuaStackObject::IsNumber() const					{  return lua_type( GetCState(), m_stackIndex ) == LUA_TNUMBER;  }


/**
	\return Returns true if the object is a string.
**/
LUAPLUS_INLINE bool LuaStackObject::IsString() const					{  return lua_isstring( GetCState(), m_stackIndex ) != 0;  }

/**
	\return Returns true if the object is a wide character string.
**/
#if LUA_WIDESTRING
LUAPLUS_INLINE bool LuaStackObject::IsWString() const				{  return lua_iswstring( GetCState(), m_stackIndex ) != 0;  }
#endif /* LUA_WIDESTRING */


/**
	\return Returns true if the object is a Lua function.
**/
LUAPLUS_INLINE bool LuaStackObject::IsFunction() const				{  return lua_isfunction( GetCState(), m_stackIndex );  }


/**
	\return Returns true if the object is none.
**/
LUAPLUS_INLINE bool LuaStackObject::IsNone() const					{  return !L  ||  lua_isnone( GetCState(), m_stackIndex );  }


/**
	\return Returns true if the object is light user data (that is, the object
		is just a pointer.
**/
LUAPLUS_INLINE bool LuaStackObject::IsLightUserData() const			{  return lua_islightuserdata( GetCState(), m_stackIndex ) != 0;  }


/**
	\return Returns true if the object is a boolean.
**/
LUAPLUS_INLINE bool LuaStackObject::IsBoolean() const				{  return lua_isboolean( GetCState(), m_stackIndex );  }


/**
	\return Returns true if the object is a thread.
**/
LUAPLUS_INLINE bool LuaStackObject::IsThread() const				{  return lua_isthread( GetCState(), m_stackIndex );  }


/**
**/
LUAPLUS_INLINE float LuaStackObject::GetFloat() const				{  return (float)lua_tonumber( GetCState(), m_stackIndex);  }
LUAPLUS_INLINE double LuaStackObject::GetDouble() const				{  return (double)lua_tonumber( GetCState(), m_stackIndex );  }
LUAPLUS_INLINE int LuaStackObject::GetInteger() const				{  return (int)lua_tonumber( GetCState(), m_stackIndex );  }
LUAPLUS_INLINE lua_Number LuaStackObject::GetNumber() const			{  return lua_tonumber( GetCState(), m_stackIndex );  }
LUAPLUS_INLINE const char* LuaStackObject::GetString() const
{
	const char* str = lua_tostring( GetCState(), m_stackIndex );
	luaplus_assert(str);
	return str;
}
#if LUA_WIDESTRING
LUAPLUS_INLINE const lua_WChar* LuaStackObject::GetWString() const
{
	const lua_WChar* str = (const lua_WChar*)lua_towstring( GetCState(), m_stackIndex );
	luaplus_assert(str);
	return str;
}
#endif /* LUA_WIDESTRING */
LUAPLUS_INLINE size_t LuaStackObject::StrLen() const				{  return lua_strlen( GetCState(), m_stackIndex );  }
LUAPLUS_INLINE lua_CFunction LuaStackObject::GetCFunction() const	{  luaplus_assert( lua_iscfunction( GetCState(), m_stackIndex ) );  return lua_tocfunction( GetCState(), m_stackIndex );  }
LUAPLUS_INLINE void* LuaStackObject::GetUserData() const			{  luaplus_assert( lua_isuserdata( GetCState(), m_stackIndex ) );  return lua_touserdata( GetCState(), m_stackIndex );  }
LUAPLUS_INLINE const void* LuaStackObject::GetLuaPointer() const	{  return lua_topointer( GetCState(), m_stackIndex );  }
LUAPLUS_INLINE void* LuaStackObject::GetLightUserData() const		{  luaplus_assert( lua_islightuserdata( GetCState(), m_stackIndex ));  return (void*)lua_touserdata( GetCState(), m_stackIndex );  }
LUAPLUS_INLINE bool LuaStackObject::GetBoolean() const				{  luaplus_assert( lua_isboolean( GetCState(), m_stackIndex )  ||  lua_isnil( GetCState(), m_stackIndex ) );  return (int)lua_toboolean( GetCState(), m_stackIndex ) != 0;  }
LUAPLUS_INLINE lua_State* LuaStackObject::GetThread() const			{  luaplus_assert( lua_isthread( GetCState(), m_stackIndex )  ||  lua_isnil( GetCState(), m_stackIndex ) );  return lua_tothread( GetCState(), m_stackIndex );  }

LUAPLUS_INLINE void LuaStackObject::Push()							{  lua_pushvalue( GetCState(), m_stackIndex );  }
// This is a dangerous function, as it can affect the stack placement of other LuaObjects.
LUAPLUS_INLINE void LuaStackObject::Pop()							{  lua_remove( GetCState(), m_stackIndex );  }

LUAPLUS_INLINE int LuaStackObject::Ref( int lock )
{
	Push();
	return luaL_ref( LuaState_to_lua_State( L ), m_stackIndex );
}

LUAPLUS_INLINE LuaStackObject LuaStackObject::GetMetaTable()
{
	lua_getmetatable( GetCState(), m_stackIndex );
	return LuaStackObject( GetState(),
		lua_gettop( GetCState() ) );
}

LUAPLUS_INLINE void LuaStackObject::SetMetaTable()					{  lua_setmetatable( GetCState(), m_stackIndex );  }
LUAPLUS_INLINE void LuaStackObject::SetMetaTable( LuaStackObject value )
{
	value.Push();
	lua_setmetatable( GetCState(), m_stackIndex );
}

LUAPLUS_INLINE void LuaStackObject::SetTable()						{  lua_settable( GetCState(), m_stackIndex );  }
LUAPLUS_INLINE size_t LuaStackObject::GetCount()					{  return lua_objlen( GetCState(), m_stackIndex );  }


/**
	Creates a table called [name] within the current LuaStackObject.

	@param name The name of the table to create.
	@param size The size of the table.
	@return Returns the object representing the newly created table.
**/
LUAPLUS_INLINE LuaStackObject LuaStackObject::CreateTable( const char* name, int narray, int lnhash )
{
	(void)narray;
	(void)lnhash;

//jj		lua_newtablesize(L, narray, lnhash);			// T
	lua_newtable( GetCState() );								// T
	lua_pushstring( GetCState(), name );						// T name
	lua_pushvalue( GetCState(), lua_gettop( GetCState() ) - 1 );	// T name T
	lua_settable( GetCState(), m_stackIndex );

	return LuaStackObject( GetState(), lua_gettop( GetCState() ) );
}

/**
	Creates a table called [key] within the current LuaStackObject.

	@param index The index of the table to create.
	@param size The size of the table.
	@return Returns the object representing the newly created table.
**/
LUAPLUS_INLINE LuaStackObject LuaStackObject::CreateTable(int index, int narray, int lnhash )
{
	(void)narray;
	(void)lnhash;

//jj		lua_newtablesize(L, narray, lnhash);			// T
	lua_newtable( GetCState() );								// T
	lua_pushnumber( GetCState(), index );						// T name
	lua_pushvalue( GetCState(), lua_gettop( GetCState() ) - 1 );	// T name T
	lua_settable( GetCState(), m_stackIndex );

	return LuaStackObject( GetState(), lua_gettop( GetCState() ) );
}

/**
	Assigns the table key [name] to nil.

	@param name The name of the object to make nil.
**/
LUAPLUS_INLINE void LuaStackObject::SetNil( const char* name )
{
	lua_pushstring( GetCState(), name );
	lua_pushnil( GetCState() );
	lua_settable( GetCState(), m_stackIndex );
}

/**
	Assigns the table key [index] to nil.

	@param key The index of the object to make nil.
**/
LUAPLUS_INLINE void LuaStackObject::SetNil( int index )
{
	lua_pushnumber( GetCState(), index );
	lua_pushnil( GetCState() );
	lua_settable( GetCState(), m_stackIndex );
}

/**
	Creates (or reassigns) the object called [name] to [value].

	@param name The name of the object to assign the value to.
	@param value The value to assign to [name].
**/
LUAPLUS_INLINE void LuaStackObject::SetBoolean( const char* name, bool value )
{
	lua_pushstring( GetCState(), name );
	lua_pushboolean( GetCState(), value );
	lua_settable( GetCState(), m_stackIndex );
}

/**
	Creates (or reassigns) the object called [index] to [value].

	@param index The index of the object to assign the value to.
	@param value The value to assign to [index].
**/
LUAPLUS_INLINE void LuaStackObject::SetBoolean( int index, bool value )
{
	lua_pushnumber( GetCState(), index );
	lua_pushboolean( GetCState(), value );
	lua_settable( GetCState(), m_stackIndex );
}

/**
	Creates (or reassigns) the object called [name] to [value].

	@param name The name of the object to assign the value to.
	@param value The value to assign to [name].
**/
LUAPLUS_INLINE void LuaStackObject::SetInteger( const char* name, int value )
{
	lua_pushstring( GetCState(), name );
	lua_pushnumber( GetCState(), value );
	lua_settable( GetCState(), m_stackIndex );
}

/**
	Creates (or reassigns) the object called [index] to [value].

	@param index The index of the object to assign the value to.
	@param value The value to assign to [index].
**/
LUAPLUS_INLINE void LuaStackObject::SetInteger( int index, int value )
{
	lua_pushnumber( GetCState(), index );
	lua_pushnumber( GetCState(), value );
	lua_settable( GetCState(), m_stackIndex );
}

/**
	Creates (or reassigns) the object called [name] to [value].

	@param name The name of the object to assign the value to.
	@param value The value to assign to [name].
**/
LUAPLUS_INLINE void LuaStackObject::SetNumber( const char* name, lua_Number value )
{
	lua_pushstring( GetCState(), name );
	lua_pushnumber( GetCState(), value );
	lua_settable( GetCState(), m_stackIndex );
}

/**
	Creates (or reassigns) the object called [index] to [value].

	@param index The index of the object to assign the value to.
	@param value The value to assign to [index].
**/
LUAPLUS_INLINE void LuaStackObject::SetNumber( int index, lua_Number value )
{
	lua_pushnumber( GetCState(), index );
	lua_pushnumber( GetCState(), value );
	lua_settable( GetCState(), m_stackIndex );
}

/**
	Creates (or reassigns) the object called [name] to [value].

	@param name The name of the object to assign the value to.
	@param value The value to assign to [name].
**/
LUAPLUS_INLINE void LuaStackObject::SetString( const char* name, const char* value )
{
	lua_pushstring( GetCState(), name );
	lua_pushstring( GetCState(), value );
	lua_settable( GetCState(), m_stackIndex );
}

/**
	Creates (or reassigns) the object called [index] to [value].

	@param index The index of the object to assign the value to.
	@param value The value to assign to [index].
**/
LUAPLUS_INLINE void LuaStackObject::SetString( int index, const char* value )
{
	lua_pushnumber( GetCState(), index );
	lua_pushstring( GetCState(), value );
	lua_settable( GetCState(), m_stackIndex );
}

/**
	Creates (or reassigns) the object called [name] to [value].

	@param name The name of the object to assign the value to.
	@param value The value to assign to [name].
**/
#if LUA_WIDESTRING
LUAPLUS_INLINE void LuaStackObject::SetWString( const char* name, const lua_WChar* value )
{
	lua_pushstring( GetCState(), name );
	lua_pushwstring( GetCState(), value );
	lua_settable( GetCState(), m_stackIndex );
}
#endif /* LUA_WIDESTRING */

/**
	Creates (or reassigns) the object called [index] to [value].

	@param index The index of the object to assign the value to.
	@param value The value to assign to [index].
**/
#if LUA_WIDESTRING
LUAPLUS_INLINE void LuaStackObject::SetWString( int index, const lua_WChar* value )
{
	lua_pushnumber( GetCState(), index );
	lua_pushwstring( GetCState(), value );
	lua_settable( GetCState(), m_stackIndex );
}
#endif /* LUA_WIDESTRING */

/**
	Creates (or reassigns) the object called [name] to [value].

	@param name The name of the object to assign the value to.
	@param value The value to assign to [name].
**/
LUAPLUS_INLINE void LuaStackObject::SetUserData( const char* name, void* value )
{
	lua_pushstring( GetCState(), name );
	(*(void **)(lua_newuserdata(GetCState(), sizeof(void *))) = (value));
	lua_settable( GetCState(), m_stackIndex );
}

/**
	Creates (or reassigns) the object called [index] to [value].

	@param index The index of the object to assign the value to.
	@param value The value to assign to [index].
**/
LUAPLUS_INLINE void LuaStackObject::SetUserData( int index, void* value )
{
	lua_pushnumber( GetCState(), index );
	(*(void **)(lua_newuserdata(GetCState(), sizeof(void *))) = (value));
	lua_settable( GetCState(), m_stackIndex );
}

/**
	Creates (or reassigns) the object called [name] to [value].

	@param name The name of the object to assign the value to.
	@param value The value to assign to [name].
**/
LUAPLUS_INLINE void LuaStackObject::SetLightUserData( int index, void* value )
{
	lua_pushnumber( GetCState(), index );
	lua_pushlightuserdata( GetCState(), value );
	lua_settable( GetCState(), m_stackIndex );
}

/**
	Creates (or reassigns) the object called [name] to [value].

	@param name The name of the object to assign the value to.
	@param value The value to assign to [name].
**/
LUAPLUS_INLINE void LuaStackObject::SetLightUserData( const char* name, void* value )
{
	lua_pushstring( GetCState(), name );
	lua_pushlightuserdata( GetCState(), value );
	lua_settable( GetCState(), m_stackIndex );
}

/**
	Creates (or reassigns) the object called [name] to [value].

	@param name The name of the object to assign the value to.
	@param value The value to assign to [name].
**/
LUAPLUS_INLINE void LuaStackObject::SetObject( const char* name, LuaStackObject& value )
{
	lua_pushstring( GetCState(), name );
	lua_pushvalue( GetCState(), value );
	lua_settable( GetCState(), m_stackIndex );
}

/**
	Creates (or reassigns) the object called [index] to [value].

	@param index The index of the object to assign the value to.
	@param value The value to assign to [index].
**/
LUAPLUS_INLINE void LuaStackObject::SetObject( int index, LuaStackObject& value )
{
	lua_pushnumber( GetCState(), index );
	lua_pushvalue( GetCState(), value );
	lua_settable( GetCState(), m_stackIndex );
}

/**
	Assuming the current object is a table, retrieves the table entry
	called [name].

	@param name The name of the entry from the current table to retrieve.
	@return Returns an LuaStackObject representing the retrieved entry.
**/
LUAPLUS_INLINE LuaStackObject LuaStackObject::GetByName( const char* name )
{
	lua_pushstring( GetCState(), name );
	lua_gettable( GetCState(), m_stackIndex );
	return LuaStackObject( GetState(), lua_gettop( GetCState() ) );
}

/**
	Assuming the current object is a table, retrieves the table entry
	at [index].

	@param index The numeric name of a table entry.
	@return Returns an LuaStackObject representing the retrieved entry.
**/
LUAPLUS_INLINE LuaStackObject LuaStackObject::GetByIndex( int index )
{
	lua_rawgeti( GetCState(), m_stackIndex, index );
	return LuaStackObject( GetState(), lua_gettop( GetCState() ) );
}

/**
	Assuming the current object is a table, retrieves the table entry
	called [name].

	@param name The name of the entry from the current table to retrieve.
	@return Returns an LuaStackObject representing the retrieved entry.
**/
LUAPLUS_INLINE LuaStackObject LuaStackObject::GetByObject( LuaStackObject& obj )
{
	lua_pushvalue( GetCState(), obj );
	lua_rawget( GetCState(), m_stackIndex );
	return LuaStackObject( GetState(), lua_gettop( GetCState() ) );
}

/**
**/
LUAPLUS_INLINE LuaStackObject LuaStackObject::operator[]( const char* name ) const
{
	lua_pushstring( GetCState(), name );
	lua_rawget( GetCState(), m_stackIndex );
	return LuaStackObject( GetState(), lua_gettop( GetCState() ) );
}

/**
**/
LUAPLUS_INLINE LuaStackObject LuaStackObject::operator[]( int index ) const
{
	lua_rawgeti( GetCState(), m_stackIndex, index );
	return LuaStackObject( GetState(), lua_gettop( GetCState() ) );
}

/**
**/
LUAPLUS_INLINE LuaStackObject LuaStackObject::operator[]( LuaStackObject& obj ) const
{
	lua_pushvalue( GetCState(), obj );
	lua_rawget( GetCState(), m_stackIndex );
	return LuaStackObject( GetState(), lua_gettop( GetCState() ) );
}

LUAPLUS_INLINE const LuaAutoObject& LuaAutoObject::operator=( const LuaStackObject& src )
{
	lua_remove( GetCState(), m_stackIndex );

	L = src.L;
	m_stackIndex = src.m_stackIndex;
	return *this;
}

LUAPLUS_INLINE LuaAutoObject::~LuaAutoObject()
{
	lua_remove( GetCState(), m_stackIndex );
}


LUAPLUS_INLINE LuaStack::LuaStack(LuaState* state)
	: L( LuaState_to_lua_State( state ) )
{
	m_numStack = lua_gettop(LuaState_to_lua_State(L));
}

	
LUAPLUS_INLINE LuaStack::LuaStack(lua_State* _L) :
    L( _L )
{
}


LUAPLUS_INLINE LuaStack::~LuaStack()
{
}


LUAPLUS_INLINE LuaStackObject LuaStack::operator[](int index)
{
//	luaplus_assert(index <= m_numStack);
	return LuaStackObject(L, index);
}



} // namespace LuaPlus

#endif // LUASTACKOBJECT_INL
