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
* Name		: ClientUserLua.cc
*
* Author	: Tony Smith <tony@perforce.com> or <tony@smee.org>
*
* Description	: Ruby bindings for the Perforce API. User interface class
* 		  for getting Perforce results into Ruby.
*
******************************************************************************/
#include <ctype.h>
#include <new>
#include "lua.h"
#include "lauxlib.h"
#include "p4/clientapi.h"
#include "p4/spec.h"
#include "p4/diff.h"
#include "p4luadebug.h"
#include "p4result.h"
#include "p4mergedata.h"
#include "ClientUserLua.h"
#include "specmgr.h"

/*******************************************************************************
* ClientUserLua - the user interface part. Gets responses from the Perforce
* server, and converts the data to Ruby form for returning to the caller.
******************************************************************************/

ClientUserLua::ClientUserLua( lua_State *_L, SpecMgr *s )
	: results( _L )
{
	L = _L;
	specMgr = s;
	debug = 0;
	inputRef = LUA_NOREF;
	mergeData = -1;
	mergeResult = -1;
	table = -1;
}

void
ClientUserLua::Reset()
{
	results.Reset();
	// Leave input alone.
}

void
ClientUserLua::Finished()
{
	// Reset input coz we should be done with it now. Keeping hold of
	// it just prevents GC from sweeping it if possible
	if ( P4LUADEBUG_CALLS && inputRef != LUA_NOREF )
		fprintf( stderr, "[P4] Cleaning up saved input\n" );

	luaL_unref( L, LUA_REGISTRYINDEX, inputRef );
	inputRef = LUA_NOREF;

	if ( table != -1 )
	{
		lua_getfield( L, table, "Finished" );
		if ( lua_isfunction( L, -1 ) )
		{
			lua_call( L, 0, 0 );
			return;
		}
		lua_pop( L, 1 );
	}
}

void
ClientUserLua::HandleError( Error *e )
{
	if( P4LUADEBUG_CALLS )
		fprintf( stderr, "[P4] HandleError()\n" );

	if( P4LUADEBUG_DATA )
	{
		StrBuf t;
		e->Fmt( t, EF_PLAIN );
		fprintf( stderr, "... [%s] %s\n", e->FmtSeverity(), t.Text() );
	}

	results.AddError( e );
}

void
ClientUserLua::OutputText( const char *data, int length )
{
	if( P4LUADEBUG_CALLS )
		fprintf( stderr, "[P4] OutputText()\n" );
	if( P4LUADEBUG_DATA )
		fprintf( stderr, "... [%d]%*s\n", length, length, data );

	results.AddOutput( data, length );
}

void
ClientUserLua::OutputInfo( char level, const char *data )
{
	if( P4LUADEBUG_CALLS )
		fprintf( stderr, "[P4] OutputInfo()\n" );
	if( P4LUADEBUG_DATA )
		fprintf( stderr, "... %s\n", data );

	results.AddOutput( data );
}

void
ClientUserLua::OutputBinary( const char *data, int length )
{
	if( P4LUADEBUG_CALLS )
		fprintf( stderr, "[P4] OutputBinary()\n" );
	if( P4LUADEBUG_DATA )
	{
		for( int l = 0; l < length; l++ )
		{
			if( l % 16 == 0 )
				fprintf( stderr, "%s... ", l ? "\n" : "" );
			fprintf( stderr, "%#hhx ", data[ l ] );
		}
	}

	//
	// Binary is just stored in a string. Since the char * version of
	// P4Result::AddOutput() assumes it can strlen() to find the length,
	// we'll make the String object here.
	//
	results.AddOutput( data, length );
}

void
ClientUserLua::OutputStat( StrDict *values )
{
	StrPtr *		spec 	= values->GetVar( "specdef" );
	StrPtr *		data 	= values->GetVar( "data" );
	StrPtr *		sf	= values->GetVar( "specFormatted" );
	StrDict *		dict	= values;
	SpecDataTable	specData;
	Error		e;

	//
	// Determine whether or not the data we've got contains a spec in one form
	// or another. 2000.1 -> 2005.1 servers supplied the form in a data variable
	// and we use the spec variable to parse the form. 2005.2 and later servers
	// supply the spec ready-parsed but set the 'specFormatted' variable to tell
	// the client what's going on. Either way, we need the specdef variable set
	// to enable spec parsing.
	//
	int			isspec	= spec && ( sf || data );

	//
	// Save the spec definition for later
	//
	if( spec )
		specMgr->AddSpecDef( cmd.Text(), spec->Text() );

	//
	// Parse any form supplied in the 'data' variable and convert it into a
	// dictionary.
	//
	if( spec && data )
	{
		// 2000.1 -> 2005.1 server's handle tagged form output by supplying the form
		// as text in the 'data' variable. We need to convert it to a dictionary
		// using the supplied spec.
		if( P4LUADEBUG_CALLS )
			fprintf( stderr, "[P4] OutputStat() - parsing form\n" );


		// Parse the form. Use the ParseNoValid() interface to prevent
		// errors caused by the use of invalid defaults for select items in
		// jobspecs.

//#if P4APIVER_ID >= 513538
		Spec s( spec->Text(), "", &e );
//#else
		//Spec s( spec->Text(), "" );
//#endif
		if( !e.Test() ) s.ParseNoValid( data->Text(), &specData, &e );
		if( e.Test() )
		{
			HandleError( &e );
			return;
		}
		dict = specData.Dict();
	}

	//
	// If what we've got is a parsed form, then we'll convert it to a P4::Spec
	// object. Otherwise it's a plain hash.
	//
	if( isspec )
	{
		if( P4LUADEBUG_CALLS )
			fprintf(stderr ,"[P4] OutputStat() - Converting to P4::Spec object\n");
		results.AddOutput( specMgr->StrDictToSpec( dict, spec ) );
		lua_pop( L, 1 );
	}
	else
	{
		if( P4LUADEBUG_CALLS )
			fprintf(stderr ,"[P4] OutputStat() - Converting to hash\n");
		results.AddOutput( specMgr->StrDictToHash( dict ) );
		lua_pop( L, 1 );
	}
}


/*
* Diff support for Ruby API. Since the Diff class only writes its output
* to files, we run the requested diff putting the output into a temporary
* file. Then we read the file in and add its contents line by line to the
* results.
*/

void
ClientUserLua::Diff( FileSys *f1, FileSys *f2, int doPage,
					char *diffFlags, Error *e )
{

	if ( P4LUADEBUG_CALLS )
		fprintf( stderr, "[P4] Diff() - comparing files\n" );

	//
	// Duck binary files. Much the same as ClientUser::Diff, we just
	// put the output into Ruby space rather than stdout.
	//
	if( !f1->IsTextual() || !f2->IsTextual() )
	{
		if ( f1->Compare( f2, e ) )
			results.AddOutput( "(... files differ ...)" );
		return;
	}

	// Time to diff the two text files. Need to ensure that the
	// files are in binary mode, so we have to create new FileSys
	// objects to do this.

	FileSys *f1_bin = FileSys::Create( FST_BINARY );
	FileSys *f2_bin = FileSys::Create( FST_BINARY );
	FileSys *t = FileSys::CreateGlobalTemp( f1->GetType() );

	f1_bin->Set( f1->Name() );
	f2_bin->Set( f2->Name() );

	{
		//
		// In its own block to make sure that the diff object is deleted
		// before we delete the FileSys objects.
		//
#ifndef OS_NEXT
		::
#endif
			Diff d;

		d.SetInput( f1_bin, f2_bin, diffFlags, e );
		if ( ! e->Test() ) d.SetOutput( t->Name(), e );
		if ( ! e->Test() ) d.DiffWithFlags( diffFlags );
		d.CloseOutput( e );

		// OK, now we have the diff output, read it in and add it to
		// the output.
		if ( ! e->Test() ) t->Open( FOM_READ, e );
		if ( ! e->Test() )
		{
			StrBuf 	b;
			while( t->ReadLine( &b, e ) )
				results.AddOutput( b.Text() );
		}
	}

	delete t;
	delete f1_bin;
	delete f2_bin;

	if ( e->Test() ) HandleError( e );
}


/*
* convert input from the user into a form digestible to Perforce. This
* involves either (a) converting any supplied hash to a Perforce form, or
* (b) running to_s on whatever we were given.
*/

void
ClientUserLua::InputData( StrBuf *strbuf, Error *e )
{
	if ( P4LUADEBUG_CALLS )
		fprintf( stderr, "[P4] InputData(). Using supplied input\n" );

	lua_rawgeti( L, LUA_REGISTRYINDEX, inputRef );		// input

	// Is it an array?
	if (lua_type( L, -1 ) == LUA_TTABLE ) {
		lua_rawgeti( L, -1, 1 );							// input input[1]
		if ( !lua_isnil( L, -1 ) ) {
			lua_getglobal( L, "table" );					// input input[1] table
			lua_getfield( L, -1, "remove" );				// input input[1] table remove
			lua_pushvalue( L, -4 );							// input input[1] table remove input
			lua_pushnumber( L, 1 );							// input input[1] table remove input 1
			lua_call( L, 2, 0 );							// input input[1] table
			lua_pop( L, 1 );								// input input[1]
		} else {
			lua_pop( L, 1 );
		}
	}

	if ( lua_isnil( L, -1 ) )
	{
//		rb_warn( "[P4] Expected user input, found none. "
//			"Missing call to P4#input()?" );
		lua_pop( L, 2 );
		return;
	}

	if ( lua_istable( L, -1 ) )
	{
		StrPtr * 	specDef = varList->GetVar( "specdef" );

		specMgr->AddSpecDef( cmd.Text(), specDef->Text() );
		specMgr->SpecToString( cmd.Text(), lua_gettop( L ), *strbuf, e );
		return;
	}

	// Convert whatever's left into a string
	strbuf->Set( lua_tostring( L, -1 ) );
}

/*
* In a script we don't really want the user to see a prompt, so we
* (ab)use the SetInput() function to allow the caller to supply the
* answer before the question is asked.
*/
void
ClientUserLua::Prompt( const StrPtr &msg, StrBuf &rsp, int noEcho, Error *e )
{
	if ( P4LUADEBUG_CALLS )
		fprintf( stderr, "[P4] Prompt(): %s\n", msg.Text() );

	InputData( &rsp, e );
}


/*
* Do a resolve. We implement a resolve by calling a block.
*/
int
ClientUserLua::Resolve( ClientMerge *m, Error *e )
{
//    if ( P4LUADEBUG_CALLS )
//        cerr << "[P4] Resolve()" << endl;

    //
    // If no resolver is defined, default to using the merger's resolve
    // if p4.input is set. Otherwise, bail out of the resolve
    //
    if( this->resolverRef == LUA_NOREF ) {
        if ( this->inputRef != LUA_NOREF ) {
            return m->Resolve( e );
        }
        else {
//            PyErr_WarnEx( PyExc_UserWarning,
//                          "[P4::Resolve] Resolve called with no resolver and no input -> skipping resolve", 1);
            return CMS_QUIT;
        }
    }

	//
	// First detect what the merger thinks the result ought to be
	//
	StrBuf t;
	MergeStatus autoMerge = m->AutoResolve( CMF_FORCE );

	// Now convert that to a string;
	switch( autoMerge )
	{
	case CMS_QUIT:	t = "q";	break;
	case CMS_SKIP:	t = "s";	break;
	case CMS_MERGED:	t = "am";	break;
	case CMS_EDIT:	t = "e";	break;
	case CMS_YOURS:	t = "ay";	break;
	case CMS_THEIRS:	t = "at";	break;
	}

	int mergeData = MkMergeInfo( m, t );					// mergeData

	lua_rawgeti( L, LUA_REGISTRYINDEX, this->resolverRef );	// mergeData resolverTable

	StrBuf 	reply;
	for( int loop=0 ; loop < 10 ; loop++ )
	{
		lua_getfield( L, -1, "resolve" );					// mergeData resolverTable resolveFunction
		if ( !lua_isfunction( L, -1 ) ) {
			lua_pop( L, 3 );								// mergeData
			return CMS_QUIT;
		}

		lua_pushvalue( L, -2 );								// mergeData resolverTable resolveFunction resolverTable
		lua_pushvalue( L, mergeData );						// mergeData resolverTable resolveFunction resolverTable mergeData
		int ret = lua_pcall( L, 2, 1, 0 );					// mergeData resolverTable result
		if ( ret != 0 ) { // exception thrown, bug out of here
			const char* err = lua_tostring( L, -1 );
			lua_pop( L, 3 );
            return CMS_QUIT;
        }

        StrBuf reply = lua_tostring( L, -1 );
		lua_pop( L, 3 );

		if( reply == "ay" ) 		return CMS_YOURS;
		else if( reply == "at" )	return CMS_THEIRS;
		else if( reply == "am" )	return CMS_MERGED;
		else if( reply == "ae" )	return CMS_EDIT;
		else if( reply == "s" )		return CMS_SKIP;
		else if( reply == "q" ) 	return CMS_QUIT;
		else {
			StrBuf warning("[P4.Resolve] Illegal response : '");
			warning << reply;
			warning << "', skipping resolve";

//			PyErr_WarnEx( PyExc_UserWarning, warning.Text(), 1);
			return CMS_QUIT;
		}
	}

//	rb_warn( "Aborting resolve after 10 attempts" );
	lua_pop( L, 2 );
	return CMS_QUIT;
}


/*
* Accept input from Ruby and convert to a StrBuf for Perforce
* purposes.  We just save what we're given here because we may not
* have the specdef available to parse it with at this time.
*/

int
ClientUserLua::SetInput( int i )
{
	if ( P4LUADEBUG_CALLS )
		fprintf( stderr, "[P4] SetInput()\n" );

	if ( inputRef != LUA_NOREF )
	    luaL_unref(L, LUA_REGISTRYINDEX, inputRef );

	lua_pushvalue( L, i );
	inputRef = luaL_ref( L, LUA_REGISTRYINDEX );
	return 1;
}


int
ClientUserLua::SetResolver( int i )
{
	if ( P4LUADEBUG_CALLS )
		fprintf( stderr, "[P4] SetResolver()\n" );

	if ( resolverRef != LUA_NOREF )
	    luaL_unref(L, LUA_REGISTRYINDEX, resolverRef );

	lua_pushvalue( L, i );
	resolverRef = luaL_ref( L, LUA_REGISTRYINDEX );
	return 1;
}


int
ClientUserLua::MkMergeInfo( ClientMerge *m, StrPtr &hint )
{
	P4MergeData* md = (P4MergeData*)lua_newuserdata( L, sizeof( P4MergeData ) );
	::new(md) P4MergeData( L, this, m, hint );

	luaL_getmetatable( L, "P4.MergeData" );
	lua_setmetatable( L, -2 );

	return lua_gettop( L );
}

