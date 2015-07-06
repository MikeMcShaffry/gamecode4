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
 * Name		: p4clientapi.h
 *
 * Author	: Tony Smith <tony@perforce.com> or <tony@smee.org>
 *
 * Description	: Ruby bindings for the Perforce API. Definitions of our
 * 		  main interface to Perforce
 *
 ******************************************************************************/


/*******************************************************************************
 * P4ClientApi class - where we register our Ruby classes and plumb together
 * the components
 ******************************************************************************/

class Enviro;
class P4ClientApi
{
public:
    P4ClientApi( lua_State *L );
    ~P4ClientApi();

    // Tagged mode - can be enabled/disabled on a per-command basis
    void Tagged(int enable );
    int	 IsTagged();

    // Returns bool, but may raise exception
    int  SetCharset( const char *c );

    // Set API level for backwards compatibility
    void SetApiLevel( int level );

    void SetClient( const char *c )	{ client.SetClient( c );	}
    void SetCwd( const char *c );
    bool SetEnv( const char *var, const char *value );
    void SetHost( const char *h )	{ client.SetHost( h );		}
    void SetKeepAlive( int keepAlive );
    void SetMaxResults( int v )		{ maxResults = v;		}
    void SetMaxScanRows( int v )	{ maxScanRows = v;		}
    void SetMaxLockTime( int v )	{ maxLockTime = v;		}
    void SetLanguage( const char *l )	{ client.SetLanguage( l );	}
    void SetPassword( const char *p )	{ client.SetPassword( p );	}
    void SetPort( const char *p )	{ client.SetPort( p );		}
    void SetProg( const char *p )	{ prog = p;			}
    void SetResolver( int resolver ){ ui.SetResolver( resolver ); }
    void SetTicketFile( const char *p );
    void SetUser( const char *u )	{ client.SetUser( u );		}
    void SetVersion( const char *v )	{ version = v;			}

    int	 GetApiLevel()				{ return apiLevel;		}
    const StrPtr &GetCharset()		{ return client.GetCharset();	}
    const StrPtr &GetClient()		{ return client.GetClient();	}
    const StrPtr &GetConfig()		{ return client.GetConfig();	}
    const StrPtr &GetCwd()			{ return client.GetCwd();	}
    const char * GetEnv( const char *v);
    const StrPtr &GetHost()			{ return client.GetHost();	}
    int GetKeepAlive();
    const StrPtr &GetLanguage()		{ return client.GetLanguage();	}
    const StrPtr &GetOs()			{ return client.GetOs();	}
    const StrPtr &GetPassword()		{ return client.GetPassword();	}
    const StrPtr &GetPort()			{ return client.GetPort();	}
    const StrPtr &GetProg()			{ return prog;			}
    int GetResolver()				{ return ui.GetResolver(); }
    const StrPtr &GetTicketFile()	{ return ticketFile;		}
    const StrPtr &GetUser()			{ return client.GetUser();	}
    const StrPtr &GetVersion()		{ return version;		}

    int		  GetMaxResults()	{ return maxResults;		}
    int		  GetMaxScanRows()	{ return maxScanRows;		}
    int		  GetMaxLockTime()	{ return maxLockTime;		}
    int		  GetServerLevel()	{ return server2;		}

    // Session management
    int Connect();		// P4Exception on error
    int Connected();		// Return true if connected and not dropped.
    int Disconnect();

    // Executing commands.
    int Run( const char *cmd, int argc, char * const *argv, int table );
    int SetInput( int input );

    // Result handling
    int GetErrors()		{ return ui.GetResults().GetErrors();}
    int GetOutput()		{ return ui.GetResults().GetOutput();}
    int GetWarnings()		{ return ui.GetResults().GetWarnings();}

    // Spec parsing
    int ParseSpec( const char * type, const char *form );
    int FormatSpec( const char *type, int hash );
    int SpecFields( const char * type );

    // Exception levels:
    //
    // 		0 - No exceptions raised
    // 		1 - Exceptions raised for errors
    // 		2 - Exceptions raised for errors and warnings
    //
    void  ExceptionLevel( int i )	{ exceptionLevel = i; 	}
    int   ExceptionLevel()		{ return exceptionLevel; }

    int Except( const char *func, Error *e );
    int Except( const char *func, const char *msg );
    int Except( const char *func, const char *msg, const char *cmd );

    //
    // Debugging support. Debug levels are:
    //
    //     1:	Debugs commands being executed
    //     2:	Debug UI method calls
    //     3:	Show garbage collection
    //
    void  SetDebug( int d );


private:

    void RunCmd(const char *cmd, ClientUser *ui, int argc, char * const *argv, int table);

private:
	// subclass KeepAlive to implement a customized IsAlive function.
	class MyKeepAlive : public KeepAlive
	{
	public:
		virtual int IsAlive();

		P4ClientApi* client;
	} ;

	lua_State *		L;
    ClientApi		client;
    ClientUserLua	ui;
    Enviro *		enviro;
    SpecMgr		specMgr;
    StrBuf		prog;
    StrBuf		version;
    StrBuf		ticketFile;
    int			depth;
    int			initCount;
    int			debug;
    int			exceptionLevel;
    int			apiLevel;
    int			server2;
    int			mode;
    int			maxResults;
    int			maxScanRows;
    int			maxLockTime;
	MyKeepAlive	cb;
	int			keepAliveRef;
};

