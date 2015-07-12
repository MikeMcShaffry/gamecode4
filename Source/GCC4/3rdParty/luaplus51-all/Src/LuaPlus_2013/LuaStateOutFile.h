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
#ifndef LUASTATEOUTFILE_H
#define LUASTATEOUTFILE_H

#include "LuaPlusInternal.h"

#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////
// namespace LuaPlus
///////////////////////////////////////////////////////////////////////////////
namespace LuaPlus
{

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Output file helper class.
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
	The DumpObject() facility uses a LuaStateOutFile derived class to
	output data to.  The LuaStateOutFile class may be derived from to enable
	an application specific method of output.
**/
class LUAPLUS_CLASS LuaStateOutFile
{
public:
	LuaStateOutFile();
	LuaStateOutFile(const char* fileName);
	virtual ~LuaStateOutFile();
	virtual bool Open( const char* fileName );
	virtual void Close();
	virtual void Print( const char* str, ... );
	bool Assign( FILE* file );
	void Indent( unsigned int indentLevel );

protected:
	FILE* m_file;
	bool m_fileOwner;
};

} // namespace LuaPlus

#endif // LUASTATEOUTFILE_H
