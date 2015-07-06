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
 * Name		: ClientUserLua.h
 *
 * Author	: Tony Smith <tony@perforce.com> or <tony@smee.org>
 *
 * Description	: Ruby bindings for the Perforce API. User interface class
 * 		  for getting Perforce results into Ruby.
 *
 ******************************************************************************/

/*******************************************************************************
 * ClientUserLua - the user interface part. Gets responses from the Perforce
 * server, and converts the data to Ruby form for returning to the caller.
 ******************************************************************************/
class SpecMgr;
class ClientUserLua : public ClientUser
{
public:
	ClientUserLua( lua_State *L, SpecMgr *s );

	// Client User methods overridden here
	virtual void	HandleError( Error *e );
	virtual void    OutputText( const char *data, int length );
	virtual void    OutputInfo( char level, const char *data );
	virtual void	OutputStat( StrDict *values );
	virtual void    OutputBinary( const char *data, int length );
	virtual void	InputData( StrBuf *strbuf, Error *e );
	virtual void	Diff( FileSys *f1, FileSys *f2, int doPage,
				char *diffFlags, Error *e );
	virtual void	Prompt( const StrPtr &msg, StrBuf &rsp, int noEcho, Error *e );

	virtual int	Resolve( ClientMerge *m, Error *e );

	void	Finished();

	// Local methods
	int		SetInput( int i );
	void	SetCommand( const char *c )	{ cmd = c;	}
	void	SetTable( int t ) { table = t;  results.SetTable( t ); }

    int		SetResolver( int i );
    int		GetResolver() { return resolverRef; }

	P4Result& GetResults()		{ return results;	}
	int	ErrorCount();
	void	Reset();

	// Debugging support
	void	SetDebug( int d )	{ debug = d; 		}

private:
	int		MkMergeInfo( ClientMerge *m, StrPtr &hint );

private:
	lua_State *	L;
	StrBuf		cmd;
	SpecMgr *	specMgr;
	P4Result	results;
	int			inputRef;
	int			resolverRef;
	int			mergeData;
	int			mergeResult;
	int		debug;
	int			table;
};

