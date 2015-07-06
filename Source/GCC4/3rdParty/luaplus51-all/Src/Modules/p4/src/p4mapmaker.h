/*******************************************************************************

Copyright (c) 2008, Perforce Software, Inc.  All rights reserved.

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
 * Name		: p4mapmaker.h
 *
 * Author	: Tony Smith <tony@perforce.com> 
 *
 * Description	: Class to encapsulate Perforce map manipulation from Ruby
 *
 ******************************************************************************/

class MapApi;
class P4MapMaker
{
public:
	P4MapMaker( lua_State *L );
	P4MapMaker( const P4MapMaker &m );

	~P4MapMaker();

	static P4MapMaker * Join( lua_State *L, P4MapMaker *l, P4MapMaker *r);

	void		Insert( int m );
	void		Insert( int l, int r );

	void		Reverse();
	void		Clear();
	int		Count();
	void		Translate( int p, int fwd = 1 );
	void		Lhs();
	void		Rhs();
	void		ToA();

	void		Inspect( StrBuf &b );

private:
	void		SplitMapping( const StrPtr &in, StrBuf &l, StrBuf &r );
	lua_State *	L;
	MapApi *	map;
};
