/*******************************************************************************

Copyright (c) 2001-2008, Perforce Software, Inc.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1.  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

2.  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL PERFORCE SOFTWARE, INC. BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/


/*******************************************************************************
 * Name		: p4result.cc
 *
 * Author	: Tony Smith <tony@perforce.com> or <tony@smee.org>
 *
 * Description	: Ruby class for holding results of Perforce commands
 *
 ******************************************************************************/

#include "lua.h"
#include "lauxlib.h"
#include "p4/clientapi.h"
#include "p4result.h"

P4Result::P4Result( lua_State *L )
	: L( L )
	, outputRef(LUA_NOREF)
	, warningsRef(LUA_NOREF)
	, errorsRef(LUA_NOREF)
	, table( -1 )
{
    Reset();
}


void
P4Result::Reset()
{
    luaL_unref(L, LUA_REGISTRYINDEX, outputRef);
    luaL_unref(L, LUA_REGISTRYINDEX, warningsRef);
    luaL_unref(L, LUA_REGISTRYINDEX, errorsRef);

    lua_newtable(L);
    outputRef = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_newtable(L);
    warningsRef = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_newtable(L);
    errorsRef = luaL_ref(L, LUA_REGISTRYINDEX);
}

void
P4Result::AddOutput( const char *msg, int length )
{
	if ( table != -1 )
	{
		lua_getfield( L, table, "OutputText" );
		if ( lua_isfunction( L, -1 ) )
		{
			lua_pushlstring(L, msg, length == -1 ? strlen( msg ) : length );
			lua_call( L, 1, 0 );
			return;
		}
		lua_pop( L, 1 );
	}

	lua_rawgeti(L, LUA_REGISTRYINDEX, outputRef);
	lua_pushnumber(L, lua_objlen(L, -1) + 1);
	lua_pushlstring(L, msg, length == -1 ? strlen( msg ) : length );
	lua_rawset(L, -3);
	lua_pop(L, 1);
}

void
P4Result::AddOutput( int out )
{
	if ( table != -1 )
	{
		lua_getfield( L, table, "OutputText" );
		if ( lua_isfunction( L, -1 ) )
		{
			lua_pushvalue( L, out );
			lua_call( L, 1, 0 );
			return;
		}
		lua_pop( L, 1 );
	}

	lua_rawgeti(L, LUA_REGISTRYINDEX, outputRef);
	lua_pushnumber(L, lua_objlen(L, -1) + 1);
	lua_pushvalue(L, out);
	lua_rawset(L, -3);
	lua_pop(L, 1);
}

void
P4Result::AddError( Error *e )
{
    StrBuf	m;
    e->Fmt( &m, EF_PLAIN );

    int s;
    s = e->GetSeverity();

    //
    // Empty and informational messages are pushed out as output as nothing
    // worthy of error handling has occurred. Warnings go into the warnings
    // list and the rest are lumped together as errors.
    //

    if ( s == E_EMPTY || s == E_INFO )
    {
		AddOutput( m.Text() );
		return;
    }

	if ( table != -1 )
	{
		lua_getfield( L, table, s == E_WARN ? "OutputWarning" : "OutputError" );
		if ( lua_isfunction( L, -1 ) )
		{
			lua_pushstring(L, m.Text() );
			lua_call( L, 1, 0 );
			return;
		}
		lua_pop( L, 1 );
	}

	if ( s == E_WARN )
		lua_rawgeti(L, LUA_REGISTRYINDEX, warningsRef);
	else
		lua_rawgeti(L, LUA_REGISTRYINDEX, errorsRef);
	lua_pushnumber(L, lua_objlen(L, -1) + 1);
	lua_pushstring(L, m.Text());
	lua_rawset(L, -3);
	lua_pop(L, 1);
}

int
P4Result::ErrorCount()
{
    return Length( errorsRef );
}

int
P4Result::WarningCount()
{
    return Length( warningsRef );
}

void
P4Result::FmtErrors( StrBuf &buf )
{
    Fmt( "[Error]: ", errorsRef, buf );
}

void
P4Result::FmtWarnings( StrBuf &buf )
{
    Fmt( "[Warning]: ", warningsRef, buf );
}


int
P4Result::Length( int ary )
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, ary);
	int len = lua_objlen(L, -1);
    lua_pop(L, 1);
    return len;
}

void
P4Result::Fmt( const char *label, int table, StrBuf &buf )
{
    buf.Clear();
    // If the array is empty, then we just return
    lua_rawgeti( L, LUA_REGISTRYINDEX, table );
    if ( lua_objlen( L, -1 ) == 0 ) return;

    // Not empty, so we'll format it.
    // This is the string we'll use to prefix each entry in the array
    StrBuf	csep;
    csep << "\n\t" << label;

    // Join the array elements together, and append the result to the buffer
	for ( int i = 1; ; ++i )
	{
		lua_rawgeti( L, -1, i );
		if ( lua_isnil( L, -1 ) )
			break;
	    buf << csep << lua_tostring( L, -1 );
		lua_pop( L, 1 );
	}
	lua_pop( L, 1 );

    return;
}

