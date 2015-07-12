///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2005 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://wwhiz.com/LuaPlus/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUASTACKTABLEITERATOR_H
#define LUASTACKTABLEITERATOR_H

#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER

///////////////////////////////////////////////////////////////////////////////
// namespace LuaPlus
///////////////////////////////////////////////////////////////////////////////
namespace LuaPlus
{

/**
	The LuaStackTableIterator class provides a far simpler, safer, and more natural
	looking table iteration method.  The iterator is not STL compliant,
	although it could easily be made so.
**/
class LUAPLUS_CLASS LuaStackTableIterator
{
public:
	LuaStackTableIterator( LuaStackObject tableObj, bool doReset = true,
			bool autoStackManagement = true );
	~LuaStackTableIterator();
	void Reset();
	void Invalidate();
	bool Next();
	bool IsValid() const;
	LuaStackTableIterator& operator++();
	operator bool() const;
	LuaStackObject GetKey();
	LuaStackObject GetValue();

protected:

private:
	/**
		Don't allow copies.  The stack will get screwed.
	**/ 
	LuaStackTableIterator& operator=( const LuaStackTableIterator& iter );
	LuaStackTableIterator( const LuaStackTableIterator& iter );

	LuaStackObject m_tableObj;				///< The table object being iterated.
	int m_startStackIndex;				///< The current starting stack index or -1 if the iterator is invalid.
	bool m_autoStackManagement;			///< Auto stack management enabled or not?
};


} // namespace LuaPlus

#endif // LUASTACKTABLEITERATOR_H
