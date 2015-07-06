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
* Name		: p4clientapi.cc
*
* Author	: Tony Smith <tony@perforce.com> or <tony@smee.org>
*
* Description	: Ruby bindings for the Perforce API. Main interface to the
* 		  Perforce API.
*
******************************************************************************/
#include "lua.h"
#include "lauxlib.h"
#include "undefdups.h"
#include "p4/clientapi.h"
#include "p4/i18napi.h"
#include "p4/enviro.h"
#include "p4/hostenv.h"
#include "p4/spec.h"
#include "p4result.h"
#include "p4luadebug.h"
#include "clientuserlua.h"
#include "specmgr.h"
#include "p4clientapi.h"


#define		M_TAGGED		0x01
#define		M_PARSE_FORMS		0x02
#define		IS_TAGGED(x)		(x & M_TAGGED )
#define		IS_PARSE_FORMS(x)	(x & M_PARSE_FORMS )


P4ClientApi::P4ClientApi( lua_State *L )
	: L( L )
	, specMgr( L )
	, ui( L, &specMgr )
	, keepAliveRef( LUA_NOREF )
{
	initCount = 0;
	debug = 0;
	server2 = 0;
	depth = 0;
	exceptionLevel = 2;
	maxResults = 0;
	maxScanRows = 0;
	maxLockTime = 0;
	apiLevel = atoi( P4Tag::l_client );
	enviro = new Enviro;
	prog = "unnamed p4lua script";
	cb.client = this;

	// Enable form parsing, and set tagged mode on by default
	mode |= M_PARSE_FORMS + M_TAGGED ;
	client.SetProtocol( "specstring", "" );

	//
	// Load any P4CONFIG file
	//
	HostEnv henv;
	StrBuf cwd;

	henv.GetCwd( cwd, enviro );
	if( cwd.Length() )
		enviro->Config( cwd );

	//
	// Load the current ticket file. Start with the default, and then
	// override it if P4TICKETS is set.
	//
	const char *t;

  	henv.GetTicketFile( ticketFile );

	if( t = enviro->Get( "P4TICKETS" ) )
		ticketFile = t;

	//
	// Load the current P4CHARSET if set.
	//
	if( client.GetCharset().Length() )
		SetCharset( client.GetCharset().Text() );
}

P4ClientApi::~P4ClientApi()
{
	if ( keepAliveRef != LUA_NOREF )
	    luaL_unref(L, LUA_REGISTRYINDEX, keepAliveRef );

	if ( initCount )
	{
		Error e;
		client.Final( &e );
		// Ignore errors
	}
	delete enviro;
}

const char *
P4ClientApi::GetEnv( const char *v)
{
	return enviro->Get( v );
}


int
P4ClientApi::GetKeepAlive()
{
	if ( keepAliveRef != LUA_NOREF )
	{
		lua_rawgeti( L, LUA_REGISTRYINDEX, keepAliveRef );
		return 1;
	}
	return 0;
}


void
P4ClientApi::SetApiLevel( int level )
{
	StrBuf	b;
	b << level;
	apiLevel = level;
	client.SetProtocol( "api", b.Text() );
}

int
P4ClientApi::SetCharset( const char *c )
{
	if( P4LUADEBUG_COMMANDS )
		fprintf( stderr, "[P4] Setting charset: %s\n", c );

	CharSetApi::CharSet cs = CharSetApi::Lookup( c );
	if( cs < 0 )
	{
		StrBuf	m;
		m = "Unknown or unsupported charset: ";
		m.Append( c );
		return Except( "P4#charset=", m.Text() );
	}
	client.SetTrans( cs, cs, cs, cs );
	client.SetCharset( c );
	return 1;
}

void
P4ClientApi::SetCwd( const char *c )
{
	client.SetCwd( c );
	enviro->Config( StrRef( c ) );
}

bool
P4ClientApi::SetEnv( const char *var, const char *value )
{
	Error e;
	enviro->Set( var, value, &e );
	return !e.Test();
}

void
P4ClientApi::SetKeepAlive( int i )
{
	if ( P4LUADEBUG_CALLS )
		fprintf( stderr, "[P4] SetResolver()\n" );

	if ( keepAliveRef != LUA_NOREF )
	    luaL_unref(L, LUA_REGISTRYINDEX, keepAliveRef );

	if ( lua_isnil( L, i ) )
	{
		keepAliveRef = LUA_NOREF;
	}
	else
	{
		lua_pushvalue( L, i );
		keepAliveRef = luaL_ref( L, LUA_REGISTRYINDEX );
	}
}

void
P4ClientApi::SetTicketFile( const char *p )
{
	client.SetTicketFile( p );
	ticketFile = p;
}

void
P4ClientApi::SetDebug( int d )
{
	debug = d;
	ui.SetDebug( d );
	specMgr.SetDebug( d );

//	if( P4LUADEBUG_RPC )
//		p4debug.SetLevel( "rpc=5" );
//	else
//		p4debug.SetLevel( "rpc=0" );
}

//
// connect to the Perforce server.
//

int
P4ClientApi::Connect()
{
	if ( P4LUADEBUG_COMMANDS )
		fprintf( stderr, "[P4] Connecting to Perforce\n" );

	if ( initCount )
	{
		lua_pushboolean( L, true );
		lua_pushstring( L, "P4#connect - Perforce client already connected!" );
		return 2;
	}

	Error	e;

	client.Init( &e );
	if ( e.Test() )
		return Except( "P4#connect", &e );

	initCount++;
	lua_pushboolean( L, true );
	return 1;
}


//
// Disconnect session
//
int
P4ClientApi::Disconnect()
{
	if ( P4LUADEBUG_COMMANDS )
		fprintf( stderr, "[P4] Disconnect\n" );

	if ( ! initCount )
	{
//		rb_warn( "P4#disconnect - not connected" );
		lua_pushboolean( L, true );
		return 1;
	}
	Error	e;
	client.Final( &e );

	// Clear the specdef cache.
	specMgr.Reset();

	initCount--;
	lua_pushboolean( L, true );
	return 1;
}

//
// Test whether or not connected
//
int
P4ClientApi::Connected()
{
	if( initCount && !client.Dropped() )
		lua_pushboolean( L, true );
	else
		lua_pushboolean( L, false );
	return 1;
}

void
P4ClientApi::Tagged( int enable )
{
	if( enable )
		mode |= M_TAGGED;
	else
		mode &= ~M_TAGGED;
}

int
P4ClientApi::IsTagged()
{
	return mode & M_TAGGED;
}


//
// Run returns the results of the command. If the client has not been
// connected, then an exception is raised but errors from Perforce
// commands are returned via the Errors() and ErrorCount() interfaces
// and not via exceptions because one failure in a command applied to many
// files would interrupt processing of all the other files if an exception
// is raised.
//

int
P4ClientApi::Run( const char *cmd, int argc, char * const *argv, int table )
{
	// Save the entire command string for our error messages. Makes it
	// easy to see where a script has gone wrong.
	StrBuf	cmdString;
	cmdString << "\"p4 " << cmd;
	for( int i = 0; i < argc; i++ )
		cmdString << " " << argv[ i ];
	cmdString << "\"";

	if ( P4LUADEBUG_COMMANDS )
		fprintf( stderr, "[P4] Executing %s\n", cmdString.Text()  );

	if ( depth )
	{
		lua_pushboolean( L, false );
		lua_pushstring( L, "Can't execute nested Perforce commands." );
		return 2;
	}

	if ( ! initCount )
		return Except( "P4#run", "not connected." );

	// Clear out any results from the previous command
	ui.Reset();

	// Tell the UI which command we're running.
	ui.SetCommand( cmd );

	ui.SetTable( table );

	depth++;
	RunCmd( cmd, &ui, argc, argv, table );
	depth--;

	P4Result &results = ui.GetResults();

	if ( results.ErrorCount() )
		return Except( "P4#run", "Errors during command execution", cmdString.Text() );

	if ( results.WarningCount() && exceptionLevel > 1 )
		return Except( "P4#run", "Warnings during command execution",cmdString.Text());

	lua_rawgeti( L, LUA_REGISTRYINDEX, results.GetOutput() );
	return 1;
}


int P4ClientApi::MyKeepAlive::IsAlive()
{
	int ret = 0;

	if ( client->keepAliveRef != LUA_NOREF ) {
		lua_rawgeti( client->L, LUA_REGISTRYINDEX, client->keepAliveRef );
		if ( lua_isfunction( client->L, -1 ) ) {
			if ( lua_pcall( client->L, 0, 1, 0 ) == 0 )
			{
				ret = lua_toboolean( client->L, -1 );
				lua_pop( client->L, 1 );
			}
		}
		else
			lua_pop( client->L, 1 );
	}
	else
		ret = 1;

	return ret;
}


//
// RunCmd is a private function to work around an obscure protocol
// bug in 2000.[12] servers. Running a "p4 -Ztag client -o" messes up the
// protocol so if they're running this command then we disconnect and
// reconnect to refresh it. For efficiency, we only do this if the
// server2 protocol is either 9 or 10 as other versions aren't affected.
//
void
P4ClientApi::RunCmd( const char *cmd, ClientUser *ui, int argc, char * const *argv, int table )
{
	client.SetProg( &prog );
	if( version.Length() )
		client.SetVersion( &version );

	if( mode & M_TAGGED )
		client.SetVar( "tag" );

	// If maxresults or maxscanrows is set, enforce them now
	if( maxResults  )	client.SetVar( "maxResults",  maxResults  );
	if( maxScanRows )	client.SetVar( "maxScanRows", maxScanRows );
	if( maxLockTime )	client.SetVar( "maxLockTime", maxLockTime );

	client.SetBreak( &cb );
	client.SetArgv( argc, argv );
	client.Run( cmd, ui );

	// Have to request server2 protocol *after* a command has been run. I
	// don't know why, but that's the way it is.

	if ( ! server2 )
	{
		StrPtr *pv = client.GetProtocol( "server2" );
		if ( pv )
			server2 = pv->Atoi();
	}

	if ( IS_TAGGED(mode) && StrRef( cmd ) == "client" &&
		server2 >= 9    && server2 <= 10  )
	{
		if ( argc && ( StrRef( argv[ 0 ] ) == "-o" ) )
		{
			if ( P4LUADEBUG_COMMANDS )
				printf( "Resetting client to avoid 2000.[12] protocol bug\n" );

			Error e;
			client.Final( &e );
			client.Init( &e );

			// Pass any errors down to the UI, so they'll get picked up.
			if ( e.Test() )
				ui->HandleError( &e );
		}
	}
}


//
// Parses a string supplied by the user into a hash. To do this we need
// the specstring from the server. We try to cache those as we see them,
// but the user may not have executed any commands to allow us to cache
// them so we may have to fetch the spec first.
//

int
P4ClientApi::ParseSpec( const char * type, const char *form )
{
	if ( !specMgr.HaveSpecDef( type ) )
	{
		StrBuf m;
		m = "No spec definition for ";
		m.Append( type );
		m.Append( " objects." );
		return Except( "P4#parse_spec", m.Text() );
	}

	// Got a specdef so now we can attempt to parse it.
	Error e;
	int v;
	v = specMgr.StringToSpec( type, form, &e );

	if ( e.Test() )
	{
		return Except( "P4#parse_spec", &e );
	}

	return 1;
}


//
// Converts a hash supplied by the user into a string using the specstring
// from the server. We may have to fetch the specstring first.
//

int
P4ClientApi::FormatSpec( const char * type, int table )
{
	if ( !specMgr.HaveSpecDef( type ) )
	{
		StrBuf m;
		m = "No spec definition for ";
		m.Append( type );
		m.Append( " objects." );
		return Except( "P4#format_spec", m.Text() );
	}

	// Got a specdef so now we can attempt to convert.
	StrBuf	buf;
	Error	e;

	specMgr.SpecToString( type, table, buf, &e );
	if( !e.Test() ) {
		lua_pushstring( L, buf.Text() );
		return 1;
	}

	StrBuf m;
	m = "Error converting hash to a string.";
	if( e.Test() ) e.Fmt( m, EF_PLAIN );
	return Except( "P4#format_spec", m.Text() );
}

//
// Returns a hash whose keys contain the names of the fields in a spec of the
// specified type. Not yet exposed to Ruby clients, but may be in future.
//
int
P4ClientApi::SpecFields( const char * type )
{
	if ( !specMgr.HaveSpecDef( type ) )
	{
		StrBuf m;
		m = "No spec definition for ";
		m.Append( type );
		m.Append( " objects." );
		return Except( "P4#spec_fields", m.Text() );
	}

	return specMgr.SpecFields( type );
}

//
// Raises an exception or returns Qfalse on bad input
//

int
P4ClientApi::SetInput( int input )
{
	if ( P4LUADEBUG_COMMANDS )
		fprintf( stderr, "[P4] Received input for next command\n" );

	if ( ! ui.SetInput( input ) )
	{
		if ( exceptionLevel )
			Except( "P4#input", "Error parsing supplied data." );
		else {
			lua_pushboolean( L, false );
			return 1;
		}
	}
	lua_pushboolean( L, true );
	return 1;
}

int
P4ClientApi::Except( const char *func, const char *msg )
{
	StrBuf	m;
	StrBuf	errors;
	StrBuf	warnings;
	int		terminate = 0;

	m << "[" << func << "] " << msg;

	// Now append any errors and warnings to the text
	ui.GetResults().FmtErrors( errors );
	ui.GetResults().FmtWarnings( warnings );

	if( errors.Length() )
	{
		m << "\n" << errors;
		terminate++;
	}

	if( exceptionLevel > 1 && warnings.Length() )
	{
		m << "\n" << warnings;
		terminate++;
	}

	if( terminate )
		m << "\n\n";

	if ( exceptionLevel )
	{
		luaL_error( L, m.Text() );
		return 0;
	}

	lua_pushnil( L );
	lua_pushstring( L, m.Text() );
	return 2;
}

int
P4ClientApi::Except( const char *func, const char *msg, const char *cmd )
{
	StrBuf m;

	m << msg;
	m << "( " << cmd << " )";
	return Except( func, m.Text() );
}

int
P4ClientApi::Except( const char *func, Error *e )
{
	StrBuf	m;

	e->Fmt( &m );
	return Except( func, m.Text() );
}

