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
 * Name		: specmgr.h
 *
 * Ruby Author	: Tony Smith <tony@perforce.com> or <tony@smee.org>
 *
 * Description	: Lua bindings for the Perforce API. Class for handling
 * 		  Perforce specs. This class provides other classes with
 * 		  generic support for parsing and formatting Perforce
 *		  specs.
 *
 ******************************************************************************/

#ifndef SPEC_MGR_H
#define SPEC_MGR_H

class StrBufDict;
class SpecMgr 
{
    public:
		SpecMgr( lua_State *L );
		~SpecMgr();
	void	SetDebug( int i )	{ debug = i; 	}

	// Clear the spec cache and revert to internal defaults
	void	Reset();

	// Add a spec to the cache
	void	AddSpecDef( const char *type, StrPtr &specDef );
	void	AddSpecDef( const char *type, const char * specDef );

	// Check that a type of spec is known.
	int	HaveSpecDef( const char *type );

	//
	// Parse routine: converts strings into Lua tables.
	//
	int		StringToSpec( const char *type, const char *spec, Error *e );

	//
	// Format routine. updates a StrBuf object with the form; 
	// that can then be converted to a Lua string where required. One
	// routine for when we know the type of spec, and one for when we
	// don't.
	//
	void	SpecToString(const char *type, int table, StrBuf &b, Error *e);

	//
	// Convert a Perforce StrDict into a Lua table.
	//
	int		StrDictToHash( StrDict *dict, int hash = -1 );

	// 
	// Convert a Perforce StrDict into a P4.Spec object. This is for
	// 2005.2 and later servers where the forms are supplied pre-parsed
	// into a dictionary - we just need to convert them. In this case,
	// we have the spec, but we don't know what type of spec it is.
	//
	int		StrDictToSpec( StrDict *dict, StrPtr *specDef );


	//
	// Return a list of the fields in a given type of spec. Return -1
	// if the spec type is not known.
	//
	int		SpecFields( const char *type );

    private:

	void	SplitKey( const StrPtr *key, StrBuf &base, StrBuf &index );
	void	InsertItem( int table, const StrPtr *var, const StrPtr *val );
	int		NewSpec( StrPtr *specDef );
	int		SpecFields( StrPtr *specDef );

    private:
	int		debug;
	StrBufDict *	specs;

	lua_State *L;
};

#endif
