///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2010 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
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
class LuaStateOutFile
{
public:
	LUAPLUS_CLASS_API LuaStateOutFile();
	LUAPLUS_CLASS_API LuaStateOutFile(const char* fileName);
	LUAPLUS_CLASS_API virtual ~LuaStateOutFile();
	LUAPLUS_CLASS_API virtual bool Open( const char* fileName );
	LUAPLUS_CLASS_API virtual void Close();
	LUAPLUS_CLASS_API virtual void Print( const char* str, ... );
	LUAPLUS_CLASS_API bool Assign( FILE* file );
	LUAPLUS_CLASS_API void Indent( unsigned int indentLevel );

protected:
	FILE* m_file;
	bool m_fileOwner;
};

} // namespace LuaPlus

#endif // LUASTATEOUTFILE_H
