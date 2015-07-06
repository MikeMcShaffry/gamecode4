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
 * Name		: p4mergedata.cc
 *
 * Author	: Tony Smith <tony@perforce.com> or <tony@smee.org>
 *
 * Description	: Class for holding merge data
 *
 ******************************************************************************/
#include "lua.h"
#include "undefdups.h"
#include "p4/clientapi.h"
#include "p4/i18napi.h"
#include "p4/strtable.h"
#include "p4/spec.h"
#include "p4result.h"
#include "p4luadebug.h"
#include "clientuserlua.h"
#include "p4mergedata.h"


P4MergeData::P4MergeData( lua_State *_L, ClientUser *ui, ClientMerge *m, StrPtr &hint )
{
	this->L = _L;
    this->debug = 0;
    this->ui = ui;
    this->merger = m;
    this->hint = hint;

    // Extract (forcibly) the paths from the RPC buffer.
    StrPtr *t;
    if( ( t = ui->varList->GetVar( "baseName" ) ) ) base = t->Text();
    if( ( t = ui->varList->GetVar( "yourName" ) ) ) yours = t->Text();
    if( ( t = ui->varList->GetVar( "theirName" ) ) ) theirs = t->Text();
    
}

int
P4MergeData::GetYourName()
{
	lua_pushstring( L, yours.Text() );
	return 1;
}

int
P4MergeData::GetTheirName()
{
	lua_pushstring( L, theirs.Text() );
	return 1;
}

int
P4MergeData::GetBaseName()
{
    lua_pushstring( L, base.Text() );
	return 1;
}


int
P4MergeData::GetYourPath()
{
    lua_pushstring( L, merger->GetYourFile()->Name() );
	return 1;
}

int
P4MergeData::GetTheirPath()
{
    lua_pushstring( L, merger->GetTheirFile()->Name() );
	return 1;
}

int
P4MergeData::GetBasePath()
{
    lua_pushstring( L, merger->GetBaseFile()->Name() );
	return 1;
}

int
P4MergeData::GetResultPath()
{
    lua_pushstring( L, merger->GetResultFile()->Name() );
	return 1;
}

int
P4MergeData::GetMergeHint()
{
    lua_pushstring( L, hint.Text() );
	return 1;
}

int
P4MergeData::RunMergeTool()
{
    Error e;
    ui->Merge( merger->GetBaseFile(), merger->GetTheirFile(),
	       merger->GetYourFile(), merger->GetResultFile(), &e );

	if( e.Test() )
		lua_pushboolean( L, false );
	else
		lua_pushboolean( L, true );
	return 1;
}


