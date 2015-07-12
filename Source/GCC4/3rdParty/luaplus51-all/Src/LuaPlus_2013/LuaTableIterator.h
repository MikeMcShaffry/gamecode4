///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2005 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://wwhiz.com/LuaPlus/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUATABLEITERATOR_H
#define LUATABLEITERATOR_H

#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER

#include "LuaPlusInternal.h"
#include "LuaObject.h"

///////////////////////////////////////////////////////////////////////////////
// namespace LuaPlus
///////////////////////////////////////////////////////////////////////////////
namespace LuaPlus
{

bool LUAPLUS_API LuaPlusH_next(LuaState* state, LuaObject* table, LuaObject* key, LuaObject* value);

/**
	The LuaTableIterator class provides a far simpler, safer, and more natural
	looking table iteration method.  The iterator is not STL compliant,
	although it could easily be made so.
**/
class LUAPLUS_CLASS LuaTableIterator
{
public:
	LuaTableIterator(LuaObject& tableObj, bool doReset = true);
	~LuaTableIterator();
	void Reset();
	void Invalidate();
	bool Next();
	bool IsValid() const;
	LuaTableIterator& operator++();
	operator bool() const;
	LuaObject& GetKey();
	LuaObject& GetValue();

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

#endif // LUATABLEITERATOR_H
