///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2010 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUAHELPER_OBJECT_H
#define LUAHELPER_OBJECT_H

#include "LuaPlusInternal.h"

///////////////////////////////////////////////////////////////////////////////
// namespace LuaPlus
///////////////////////////////////////////////////////////////////////////////
namespace LuaPlus
{

/**
**/
namespace LuaHelper
{
	/**
		Attempts retrieval of the value from the passed in LuaObject.

		\param obj The LuaObject representing a table to attempt retrieval
			of the value from.
		\param key The lookup key for the value.  The key may be of number,
			string, or LuaObject type.
		\param require If set to true, the key must exist in the table and
			must be of the requested type.  If it doesn't exist or isn't of
			the type, an assertion fires.  Otherwise, defaultValue is 
			returned.
		\param defaultValue The default value to be returned if require is
			not true and the key doesn't exist or the found value is not
			of the right type.
		\return Returns the value found or the defaultValue.
	**/
	LUAPLUS_API bool GetBoolean( LuaObject& obj, int key, bool require = true, bool defaultValue = false );
	LUAPLUS_API bool GetBoolean( LuaObject& obj, const char* key, bool require = true, bool defaultValue = false );


	/**
		Attempts retrieval of the value from the passed in LuaObject.

		\param obj The LuaObject representing a table to attempt retrieval
			of the value from.
		\param key The lookup key for the value.  The key may be of number,
			string, or LuaObject type.
		\param require If set to true, the key must exist in the table and
			must be of the requested type.  If it doesn't exist or isn't of
			the type, an assertion fires.  Otherwise, defaultValue is 
			returned.
		\param defaultValue The default value to be returned if require is
			not true and the key doesn't exist or the found value is not
			of the right type.
		\return Returns the value found or the defaultValue.
	**/
	LUAPLUS_API int GetInteger( LuaObject& obj, int key, bool require = true, int defaultValue = -1 );
	LUAPLUS_API int GetInteger( LuaObject& obj, const char* key, bool require = true, int defaultValue = -1 );


	/**
		Attempts retrieval of the value from the passed in LuaObject.

		\param obj The LuaObject representing a table to attempt retrieval
			of the value from.
		\param key The lookup key for the value.  The key may be of number,
			string, or LuaObject type.
		\param require If set to true, the key must exist in the table and
			must be of the requested type.  If it doesn't exist or isn't of
			the type, an assertion fires.  Otherwise, defaultValue is 
			returned.
		\param defaultValue The default value to be returned if require is
			not true and the key doesn't exist or the found value is not
			of the right type.
		\return Returns the value found or the defaultValue.
	**/
	LUAPLUS_API float GetFloat( LuaObject& obj, int key, bool require = true, float defaultValue = -1.0f );
	LUAPLUS_API float GetFloat( LuaObject& obj, const char* key, bool require = true, float defaultValue = -1.0f );


	/**
		Attempts retrieval of the value from the passed in LuaObject.

		\param obj The LuaObject representing a table to attempt retrieval
			of the value from.
		\param key The lookup key for the value.  The key may be of number,
			string, or LuaObject type.
		\param require If set to true, the key must exist in the table and
			must be of the requested type.  If it doesn't exist or isn't of
			the type, an assertion fires.  Otherwise, defaultValue is 
			returned.
		\param defaultValue The default value to be returned if require is
			not true and the key doesn't exist or the found value is not
			of the right type.
		\return Returns the value found or the defaultValue.
	**/
	LUAPLUS_API void* GetLightUserData( LuaObject& obj, int key, bool require = true, void* defaultValue = NULL );
	LUAPLUS_API void* GetLightUserData( LuaObject& obj, const char* key, bool require = true, void* defaultValue = NULL );


	/**
		Attempts retrieval of the value from the passed in LuaObject.

		\param obj The LuaObject representing a table to attempt retrieval
			of the value from.
		\param key The lookup key for the value.  The key may be of number,
			string, or LuaObject type.
		\param require If set to true, the key must exist in the table and
			must be of the requested type.  If it doesn't exist or isn't of
			the type, an assertion fires.  Otherwise, defaultValue is 
			returned.
		\param defaultValue The default value to be returned if require is
			not true and the key doesn't exist or the found value is not
			of the right type.
		\return Returns the value found or the defaultValue.
	**/
	LUAPLUS_API const char* GetString( LuaObject& obj, int key, bool require = true, const char* defaultValue = "" );
	LUAPLUS_API const char* GetString( LuaObject& obj, const char* key, bool require = true, const char* defaultValue = "" );


	/**
		Attempts retrieval of the obj from the passed in LuaObject.

		\param obj The LuaObject representing a table to attempt retrieval
			of the value from.
		\param key The lookup key for the value.  The key may be of number,
			string, or LuaObject type.
		\param require If set to true, the key must exist in the table and
			must be of the requested type.  If it doesn't exist or isn't of
			the type, an assertion fires.  Otherwise, defaultValue is 
			returned.
		\return Returns the object found.
	**/
	LUAPLUS_API LuaObject GetTable( LuaObject& obj, int key, bool require = true );
	LUAPLUS_API LuaObject GetTable( LuaObject& obj, const char* key, bool require = true );
} // namespace LuaHelper

} // namespace LuaPlus


#endif // LUAHELPER_OBJECT_H
