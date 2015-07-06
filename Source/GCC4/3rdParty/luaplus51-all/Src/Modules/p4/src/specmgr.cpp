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
* Name		: specmgr.cc
*
* Author	: Tony Smith <tony@perforce.com> or <tony@smee.org>
*
* Description	: Ruby bindings for the Perforce API. Class for handling
* 		  Perforce specs. This class provides other classes with
* 		  generic support for parsing and formatting Perforce
*		  specs.
*
******************************************************************************/
#include <ctype.h>
#include "lua.h"
#include "lauxlib.h"
#include "undefdups.h"
#include "p4/clientapi.h"
#include "p4/strops.h"
#include "p4/spec.h"
#include "p4/strtable.h"
#include "p4luadebug.h"
#include "specmgr.h"

struct defaultspec {
	const char *type;
	const char *spec;
} speclist[] = {

	{
		"branch",
			"Branch;code:301;rq;ro;fmt:L;len:32;;"
			"Update;code:302;type:date;ro;fmt:L;len:20;;"
			"Access;code:303;type:date;ro;fmt:L;len:20;;"
			"Owner;code:304;fmt:R;len:32;;"
			"Description;code:306;type:text;len:128;;"
			"Options;code:309;type:line;len:32;val:"
			"unlocked/locked;;"
			"View;code:311;type:wlist;words:2;len:64;;"
	},
	{
		"change",
			"Change;code:201;rq;ro;fmt:L;seq:1;len:10;;"
			"Date;code:202;type:date;ro;fmt:R;seq:3;len:20;;"
			"Client;code:203;ro;fmt:L;seq:2;len:32;;"
			"User;code:204;ro;fmt:L;seq:4;len:32;;"
			"Status;code:205;ro;fmt:R;seq:5;len:10;;"
			"Description;code:206;type:text;rq;seq:6;;"
			"JobStatus;code:207;fmt:I;type:select;seq:8;;"
			"Jobs;code:208;type:wlist;seq:7;len:32;;"
			"Files;code:210;type:llist;len:64;;"
		},
		{
			"client",
				"Client;code:301;rq;ro;seq:1;len:32;;"
				"Update;code:302;type:date;ro;seq:2;fmt:L;len:20;;"
				"Access;code:303;type:date;ro;seq:4;fmt:L;len:20;;"
				"Owner;code:304;seq:3;fmt:R;len:32;;"
				"Host;code:305;seq:5;fmt:R;len:32;;"
				"Description;code:306;type:text;len:128;;"
				"Root;code:307;rq;type:line;len:64;;"
				"AltRoots;code:308;type:llist;len:64;;"
				"Options;code:309;type:line;len:64;val:"
				"noallwrite/allwrite,noclobber/clobber,nocompress/compress,"
				"unlocked/locked,nomodtime/modtime,normdir/rmdir;;"
				"SubmitOptions;code:313;type:select;fmt:L;len:25;val:"
				"submitunchanged/submitunchanged+reopen/revertunchanged/"
				"revertunchanged+reopen/leaveunchanged/leaveunchanged+reopen;;"
				"LineEnd;code:310;type:select;fmt:L;len:12;val:"
				"local/unix/mac/win/share;;"
				"View;code:311;type:wlist;words:2;len:64;;"
		},
		{
			"depot",
				"Depot;code:251;rq;ro;len:32;;"
				"Owner;code:252;len:32;;"
				"Date;code:253;type:date;ro;len:20;;"
				"Description;code:254;type:text;len:128;;"
				"Type;code:255;rq;len:10;;"
				"Address;code:256;len:64;;"
				"Suffix;code:258;len:64;;"
				"Map;code:257;rq;len:64;;"
			},
			{
				"group",
					"Group;code:401;rq;ro;len:32;;"
					"MaxResults;code:402;type:word;len:12;;"
					"MaxScanRows;code:403;type:word;len:12;;"
					"MaxLockTime;code:407;type:word;len:12;;"
					"Timeout;code:406;type:word;len:12;;"
					"Subgroups;code:404;type:wlist;len:32;opt:default;;"
					"Owners;code:408;type:wlist;len:32;opt:default;;"
					"Users;code:405;type:wlist;len:32;opt:default;;"
			},
			{
				"job",
					"Job;code:101;rq;len:32;;"
					"Status;code:102;type:select;rq;len:10;"
					"pre:open;val:open/suspended/closed;;"
					"User;code:103;rq;len:32;pre:$user;;"
					"Date;code:104;type:date;ro;len:20;pre:$now;;"
					"Description;code:105;type:text;rq;pre:$blank;;"
				},
				{
					"label",
						"Label;code:301;rq;ro;fmt:L;len:32;;"
						"Update;code:302;type:date;ro;fmt:L;len:20;;"
						"Access;code:303;type:date;ro;fmt:L;len:20;;"
						"Owner;code:304;fmt:R;len:32;;"
						"Description;code:306;type:text;len:128;;"
						"Options;code:309;type:line;len:64;val:"
						"unlocked/locked;;"
						"Revision;code:312;type:word;words:1;len:64;;"
						"View;code:311;type:wlist;len:64;;"
				},
				{
					"license",
						"License;code:451;len:32;;"
						"License-Expires;code:452;len:10;;"
						"Support-Expires;code:453;len:10;;"
						"Customer;code:454;type:line;len:128;;"
						"Application;code:455;len:32;;"
						"IPaddress;code:456;len:24;;"
						"Platform;code:457;len:32;;"
						"Clients;code:458;len:8;;"
						"Users;code:459;len:8;;"
					},
					{
						"protect",
							"Protections;code:501;type:wlist;words:5;opt:default;len:64;;"
					},
					{
						"spec",
							"Fields;code:351;type:wlist;words:5;rq;;"
							"Words;code:352;type:wlist;words:2;;"
							"Formats;code:353;type:wlist;words:3;;"
							"Values;code:354;type:wlist;words:2;;"
							"Presets;code:355;type:wlist;words:2;;"
							"Comments;code:356;type:text;;"
						},
						{
							"triggers",
								"Triggers;code:551;type:wlist;words:4;len:64;opt:default;"
						},
						{
							"typemap",
								"TypeMap;code:601;type:wlist;words:2;len:64;opt:default;"
							},
							{
								"user",
									"User;code:651;rq;ro;seq:1;len:32;;"
									"Email;code:652;fmt:R;rq;seq:3;len:32;;"
									"Update;code:653;fmt:L;type:date;ro;seq:2;len:20;;"
									"Access;code:654;fmt:L;type:date;ro;len:20;;"
									"FullName;code:655;fmt:R;type:line;rq;len:32;;"
									"JobView;code:656;type:line;len:64;;"
									"Password;code:657;len:32;;"
									"Reviews;code:658;type:wlist;len:64;;"
							},
							{ 0, 0 }
};


SpecMgr::SpecMgr( lua_State *L )
	: L( L )
{
	debug = 0;
	specs = 0;
	Reset();
}

SpecMgr::~SpecMgr()
{
	delete specs;
}

void
SpecMgr::AddSpecDef( const char *type, StrPtr &specDef )
{
	if( specs->GetVar( type ) )
		specs->RemoveVar( type );
	specs->SetVar( type, specDef );
}

void
SpecMgr::AddSpecDef( const char *type, const char *specDef )
{
	if( specs->GetVar( type ) )
		specs->RemoveVar( type );
	specs->SetVar( type, specDef );
}


void
SpecMgr::Reset()
{
	delete specs;
	specs = new StrBufDict;

	for( struct defaultspec *sp = &speclist[ 0 ]; sp->type; sp++ )
		AddSpecDef( sp->type, sp->spec );

}

int
SpecMgr::HaveSpecDef( const char *type )
{
	return specs->GetVar( type ) != 0;
}

//
// Convert a Perforce StrDict into a Lua table. Convert multi-level 
// data (Files0, Files1 etc. ) into (nested) array members of the hash. 
//

int
SpecMgr::StrDictToHash( StrDict *dict, int hash )
{
	StrRef	var, val;
	int		i;

	if( hash == -1 )
	{
		lua_newtable( L );
		hash = lua_gettop( L );
	}

	for ( i = 0; dict->GetVar( i, var, val ); i++ )
	{
		if ( var == "specdef" || var == "func" || var == "specFormatted" )
			continue;

		InsertItem( hash, &var, &val );
	}
	return hash;
}

//
// Convert a Perforce StrDict into a P4::Spec object
//

int
SpecMgr::StrDictToSpec( StrDict *dict, StrPtr *specDef )
{
	int spec = NewSpec( specDef );
	return StrDictToHash( dict, spec );
}

int
SpecMgr::StringToSpec( const char *type, const char *form, Error *e )
{

	SpecDataTable	specData;
	StrPtr *		specDef = specs->GetVar( type );
	Spec		s( specDef->Text(), "", e );

	if( !e->Test() ) 
		s.ParseNoValid( form, &specData, e );

	if ( e->Test() )
		return -1;

	return StrDictToSpec( specData.Dict(), specDef );
}


//
// Format routine. updates a StrBuf object with the form content. 
// The StrBuf can then be converted to a Lua string where required.
//
void
SpecMgr::SpecToString( const char *type, int table, StrBuf &b, Error *e )
{

	StrBuf	buf;
	SpecMgr	m( L );
	StrPtr *	specDef = specs->GetVar( type );
	if ( !specDef )
	{
		e->Set( E_FAILED, "No specdef available. Cannot convert table to a "
			"Perforce form" );
		return;
	}

	SpecDataTable	specData;
	Spec		s( specDef->Text(), "", e );

	if( e->Test() ) return;

	lua_pushnil( L );  /* first key */
	while ( lua_next( L, table ) != 0 )
	{
		StrBuf	keyStr;
		lua_pushvalue( L, -2 );
		keyStr.Set( lua_tostring( L, -1 ) );
		lua_pop( L, 1 );

		if ( lua_istable( L, -1 ) )
		{
			for ( int idx2 = 1; ; ++idx2 )
			{
				lua_rawgeti( L, -1, idx2 );
				if ( lua_isnil( L, -1 ) )
				{
					lua_pop( L, 1 );
					break;
				}

				StrBuf	tKey;
				tKey.Alloc( 32 );
				sprintf( tKey.Text(), "%s%d", keyStr.Text(), idx2 - 1 );
				specData.Dict()->SetVar( tKey.Text(), lua_tostring( L, -1 ) );

				if( P4LUADEBUG_DATA )
					fprintf( stderr, "... %s -> %s\n", tKey.Text(), lua_tostring( L, -1 ) );

				lua_pop( L, 1 );
			}
		}
		else
		{
			specData.Dict()->SetVar( keyStr.Text(), lua_tostring( L, -1 ) );
			if( P4LUADEBUG_DATA )
				fprintf( stderr, "... %s -> %s\n", keyStr.Text(), lua_tostring( L, -1 ) );
		}

		lua_pop( L, 1 );  /* removes 'value'; keeps 'key' for next iteration */
	}

	s.Format( &specData, &b );
}

//
// This method returns a table describing the valid fields in the spec. To
// make it easy on our users, we map the lowercase name to the name defined
// in the spec. Thus, the users can always user lowercase, and if the field
// should be in mixed case, it will be. See P4::Spec::method_missing
//

int
SpecMgr::SpecFields( const char *type )
{
	return SpecFields( specs->GetVar( type ) );
}

int
SpecMgr::SpecFields( StrPtr *specDef )
{
	if( !specDef ) return 0;

	//
	// There's no trivial way to do this using the API (and get it right), so
	// for now, we parse the string manually. We're ignoring the type of 
	// the field, and any constraints it may be under; what we're interested
	// in is solely the field name
	//
	lua_newtable( L );
	int table = lua_gettop( L );

	const char *b, *e;
	const char *sep = ";";
	const char *fsep = ";;";
	const char *seek = sep;

	for( e = b = specDef->Text(); e && b ; )
	{
		e = strstr( b, seek );
		if( e && seek == sep )
		{
			StrBuf	k;
			k.Set( b, e - b );

			StrBuf v( k );
			StrOps::Lower( k );

			lua_pushstring( L, v.Text() );
			lua_setfield( L, -2, k.Text() );
			b = ++e;
			seek = fsep;
		}
		else if( e )
		{
			b = e += 2;
			seek = sep;
		}
	}
	return 1;
}

//
// Split a key into its base name and its index. i.e. for a key "how1,0"
// the base name is "how" and they index is "1,0". We work backwards from
// the end of the key looking for the first char that is neither a
// digit, nor a comma.
//

void
SpecMgr::SplitKey( const StrPtr *key, StrBuf &base, StrBuf &index )
{
	int i = 0;

	base = *key;
	index = "";
	for ( i = key->Length(); i; i-- )
	{
		char	prev = (*key)[ i-1 ];
		if ( !isdigit( prev ) && prev != ',' )
		{
			base.Set( key->Text(), i );
			index.Set( key->Text() + i );
			break;
		}
	}
}

//
// Insert an element into the response structure. The element may need to
// be inserted into an array nested deeply within the enclosing table.
//

void
SpecMgr::InsertItem( int table, const StrPtr *var, const StrPtr *val )
{
	StrBuf	base, index;
	StrRef	comma( "," );

	SplitKey( var, base, index );

	// If there's no index, then we insert into the top level table
	// but if the key is already defined then we need to rename the key. This
	// is probably one of those special keys like otherOpen which can be
	// both an array element and a scalar. The scalar comes last, so we
	// just rename it to "otherOpens" to avoid trashing the previous key
	// value
	if ( index == "" )
	{
		StrBuf key( *var );
		lua_pushstring( L, key.Text() );				// key
		lua_rawget( L, table );							// table[key]
		if ( lua_isstring( L, -1 ) )
			key << "s";
		lua_pop( L, 1 );								// (none)

		if( P4LUADEBUG_DATA )
			fprintf( stderr, "... %s -> %s\n", key.Text(), val->Text() );

		lua_pushstring( L, key.Text() );				// key
		lua_pushstring( L, val->Text() );				// key val
		lua_rawset( L, table );							// (none)
		return;
	}

	//
	// Get or create the parent array from the table.
	//
	lua_pushstring( L, base.Text() );					// base
	lua_rawget( L, table );								// table[base]
	if ( lua_isnil( L, -1 ) )
	{
		lua_pop( L, 1 );								// (none)
		lua_newtable( L );								// newtable
		lua_pushstring( L, base.Text() );				// newtable base
		lua_pushvalue( L, -2 );							// newtable base newtable
		lua_rawset( L, table );							// newtable
	}
	else if ( !lua_istable( L, -1 ) )
	{
		//
		// There's an index in our var name, but the name is already defined 
		// and the value it contains is not an array. This means we've got a 
		// name collision. This can happen in 'p4 diff2' for example, when 
		// one file gets 'depotFile' and the other gets 'depotFile2'. In 
		// these cases it makes sense to keep the structure flat so we
		// just use the raw variable name.
		//
		if( P4LUADEBUG_DATA )
			fprintf( stderr, "... %s -> %s\n", var->Text(), val->Text() );

		lua_pop( L, 1 );								// (none)
		lua_pushstring( L, val->Text() );				// val
		lua_setfield( L, table, var->Text() );			// (none)
		return;
	}

	// The index may be a simple digit, or it could be a comma separated
	// list of digits. For each "level" in the index, we need a containing
	// array.
	if( P4LUADEBUG_DATA )
		fprintf( stderr, "... %s -> [", base.Text() );

	for( const char *c = 0 ; ( c = index.Contains( comma ) ); )
	{
		StrBuf	level;
		level.Set( index.Text(), c - index.Text() );
		index.Set( c + 1 );

		// Found another level so we need to get/create a nested array
		// under the current entry. We use the level as an index so that
		// missing entries are left empty deliberately.

		lua_rawgeti( L, -1, level.Atoi() + 1 );			// newtable newtable[level.Atoi()+1]
		if ( lua_isnil( L, -1 ) )
		{
			lua_pop( L, 1 );							// newtable
			lua_newtable( L );							// newtable subtable
			lua_pushvalue( L, -1 );						// newtable subtable subtable
			lua_rawseti( L, -3, level.Atoi() + 1 );		// newtable[level.Atoi()+1] = subtable // newtable subtable
			lua_remove( L, -2 );						// subtable
		}
		if( P4LUADEBUG_DATA )
			fprintf( stderr, "%s][", level.Text() );
	}

	int pos = index.Atoi() + 1;

	if( P4LUADEBUG_DATA )
		fprintf( stderr, "%d] = %s\n", pos, val->Text() );

	lua_pushstring( L, val->Text() );					// newtable val
	lua_rawseti( L, -2, pos );							// newtable
	lua_pop( L, 1 );									// (none)
}


#define P4_SPEC_METATABLE "p4.Spec"

static int p4_spec_tostring(lua_State *L) {
	lua_pushstring( L, P4_SPEC_METATABLE );
	return 1;
}

static int p4_spec_index(lua_State *L) {
	const char* key = luaL_checklstring( L, 2, NULL );
	if ( key[0] != '_' )
		luaL_argerror( L, 2, "spec attribute not found" );
	lua_pushstring( L, key + 1 );
	lua_rawget( L, 1 );
	if ( !lua_isnil( L, -1 ) )
		return 1;
	lua_pushstring( L, key + 1 );
	lua_rawget( L, lua_upvalueindex( 1 ) );
	if ( lua_isnil( L, -1 ) )
		luaL_argerror( L, 2, "spec attribute not found" );
	lua_rawget( L, 1 );
	if ( !lua_isnil( L, -1 ) )
		return 1;
	return 0;
}


static int p4_spec_newindex(lua_State *L) {
	const char* key = luaL_checklstring( L, 2, NULL );
	if ( key[0] != '_' )
		luaL_argerror( L, 2, "Illegal field" );
	size_t keyLen = strlen( key + 1 );
	char* lowerKey = new char[ keyLen + 1 ];
	char* ptr;
	char* end;
	strcpy( lowerKey, key + 1 );
	ptr = lowerKey;
	end = ptr + keyLen;
	while ( ptr != end ) {
		*ptr = (char)tolower( *ptr );
		++ptr;
	}
	lua_pushstring( L, lowerKey );
	lua_rawget( L, lua_upvalueindex( 1 ) );
	if ( lua_isnil( L, -1 ) )
		luaL_argerror( L, 2, "Illegal field" );
	lua_pushvalue( L, 3 );
	lua_rawset( L, 1 );
	return 0;
}


static int p4_spec_create_metatable( lua_State *L )
{
	luaL_newmetatable( L, P4_SPEC_METATABLE );			// specfields metatable
//	lua_pushliteral( L, "__tostring" );
//	lua_pushcfunction( L, p4_spec_tostring );
//	lua_settable(L, -3);
	lua_pushliteral( L, "__index" );					// specfields metatable __index
	lua_pushvalue( L, -3 );								// specfields metatable __index specfields
	lua_pushcclosure( L, p4_spec_index, 1 );			// specfields metatable __index indexfunc
	lua_settable( L, -3 );								// specfields metatable
	lua_pushliteral( L, "__newindex" );					// specfields metatable __newindex
	lua_pushvalue( L, -3 );								// specfields metatable __newindex specfields
	lua_pushcclosure( L, p4_spec_newindex, 1 );			// specfields metatable __newindex specfields newindexfunc
	lua_settable( L, -3 );								// specfields metatable
	lua_remove( L, -2 );
	return 0;
}


//
// Create a new P4::Spec object and return it.
//

int
SpecMgr::NewSpec( StrPtr *specDef )
{
	lua_newtable( L );

	luaL_getmetatable( L, P4_SPEC_METATABLE );
	if ( lua_isnil( L, -1 ) )
	{
		lua_pop( L, 1 );
		SpecFields( specDef );
		p4_spec_create_metatable( L );
	}
	lua_setmetatable(L, -2);

	return lua_gettop( L );
}
