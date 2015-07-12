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
#ifndef LUAPLUSFUNCTIONS_H
#define LUAPLUSFUNCTIONS_H

namespace LuaPlus {
class LuaState;

void ScriptFunctionsRegister( LuaPlus::LuaState* state );
} // namespace LuaPlus

#endif // LUAPLUSFUNCTIONS_H
