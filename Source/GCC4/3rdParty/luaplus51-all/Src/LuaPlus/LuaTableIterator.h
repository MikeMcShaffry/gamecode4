///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2010 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUATABLEITERATOR_H
#define LUATABLEITERATOR_H

#include "LuaPlusInternal.h"
#include "LuaObject.h"

#if LUAPLUS_EXTENSIONS

///////////////////////////////////////////////////////////////////////////////
// namespace LuaPlus
///////////////////////////////////////////////////////////////////////////////
namespace LuaPlus
{

/**
	The LuaTableIterator class provides a far simpler, safer, and more natural
	looking table iteration method.
**/
class LuaTableIterator
{
public:
	LUAPLUS_CLASS_API LuaTableIterator(const LuaObject& tableObj, bool doReset = true);
	LUAPLUS_CLASS_API ~LuaTableIterator();
	LUAPLUS_CLASS_API void Reset();
	LUAPLUS_CLASS_API void Invalidate();
	LUAPLUS_CLASS_API bool Next();
	LUAPLUS_CLASS_API bool IsValid() const;
	LUAPLUS_CLASS_API LuaTableIterator& operator++();
	LUAPLUS_CLASS_API operator bool() const;
	LUAPLUS_CLASS_API LuaObject& GetKey();
	LUAPLUS_CLASS_API LuaObject& GetValue();

protected:

private:
	/**
		Don't allow copies.
	**/
	LuaTableIterator& operator=( const LuaTableIterator& iter );
	LuaTableIterator( const LuaTableIterator& iter );

	LuaObject m_keyObj;
	LuaObject m_valueObj;
	LuaObject m_tableObj;				///< The table object being iterated.
	bool m_isDone;
};

} // namespace LuaPlus

#endif // LUAPLUS_EXTENSIONS

#endif // LUATABLEITERATOR_H
