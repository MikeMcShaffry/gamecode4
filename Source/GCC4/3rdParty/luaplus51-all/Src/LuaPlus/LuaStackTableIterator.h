///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2010 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUASTACKTABLEITERATOR_H
#define LUASTACKTABLEITERATOR_H

#include "LuaStackObject.h"

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
class LuaStackTableIterator
{
public:
	LuaStackTableIterator(LuaStackObject& tableObj, bool doReset = true, bool autoStackManagement = true);
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
	LuaStackTableIterator& operator=(const LuaStackTableIterator& iter);
	LuaStackTableIterator(const LuaStackTableIterator& iter);

	lua_State* L;
	int stackIndex;
	int startStackIndex;				///< The current starting stack index or -1 if the iterator is invalid.
	bool autoStackManagement;			///< Auto stack management enabled or not?
};

} // namespace LuaPlus

#ifdef LUAPLUS_ENABLE_INLINES
#include "LuaStackTableIterator.inl"
#endif // LUAPLUS_ENABLE_INLINES

#endif // LUASTACKTABLEITERATOR_H
