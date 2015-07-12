///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2005 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://wwhiz.com/LuaPlus/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER
#ifndef LUAHELPER_H
#define LUAHELPER_H

#include "LuaHelper_Object.h"
#include "LuaHelper_StackObject.h"

namespace LuaPlus
{
	namespace LuaHelper
	{
		LUAPLUS_API void MergeObjects(LuaObject& mergeTo, LuaObject& mergeFrom, bool replaceDuplicates);
	}
}

#endif // LUAHELPER_H
