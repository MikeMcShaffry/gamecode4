///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2010 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef BUILDING_LUAPLUS
#define BUILDING_LUAPLUS
#endif
#include "LuaLink.h"
LUA_EXTERN_C_BEGIN
#include "src/lobject.h"
LUA_EXTERN_C_END
#include "LuaPlus.h"

namespace LuaPlus {

LuaStateOutFile::LuaStateOutFile() :
    m_file( NULL ),
    m_fileOwner( false )
{
}


LuaStateOutFile::LuaStateOutFile(const char* fileName) :
    m_file( NULL ),
    m_fileOwner( false )
{
	Open(fileName);
}


LuaStateOutFile::~LuaStateOutFile()
{
	if ( m_file  &&  m_fileOwner )
		fclose( m_file );
}


bool LuaStateOutFile::Open( const char* fileName )
{
	Close();

    if (fileName[0] == '+')
	    m_file = fopen( fileName + 1, "a+b" );
    else
	    m_file = fopen( fileName, "wb" );
	m_fileOwner = true;

	return m_file != NULL;
}


void LuaStateOutFile::Close()
{
	if ( m_file  &&  m_fileOwner )
		fclose( m_file );
}


void LuaStateOutFile::Print( const char* str, ... )
{
	char message[ 800 ];
	va_list arglist;

	va_start( arglist, str );
	vsprintf( message, str, arglist );
	va_end( arglist );

	fputs( message, m_file );
}


bool LuaStateOutFile::Assign( FILE* file )
{
	m_file = file;
	m_fileOwner = false;

	return true;
}

} // namespace LuaPlus
