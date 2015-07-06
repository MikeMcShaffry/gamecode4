#include "LuaRemoteDebuggingServer/LuaRemoteDebuggingServer.h"
#include "LuaPlus/LuaPlus.h"
#include <ctype.h>
#include <assert.h>
#pragma warning(disable: 4702)
#include "LuaNetworkHelpers.h"
#include "LuaNetworkServer.h"
extern "C" {
#include "LuaPlus/src/lua.h"
#include "LuaPlus/src/lauxlib.h"
}

#pragma warning( disable: 4100 )

using namespace LuaPlus;
using namespace LuaDebugger;

static const char* LUA_DEBUGGER_SOCKET_NAME = "LuaDebugger";
static int m_nestedCalls = 0;
static LuaState* m_networkLuaState;
static LuaState* curDebuggingState;
static SimpleString curDebuggingStateName;

struct LuaStateInfo
{
	LuaState* state;
	SimpleString stateName;
};
typedef SimpleList<LuaStateInfo> LuaStateInfoList;
LuaStateInfoList luaStateInfoList;

LuaNetworkServer* m_networkServer;
static bool m_connected = false;

typedef SimpleList<SimpleString> ExpandIdentArray;

struct WatchInfo
{
	SimpleString m_ident;
	ExpandIdentArray m_expandIdents;
};

typedef SimpleList< WatchInfo > WatchList;
static WatchList m_watchList;

static ExpandIdentArray m_localExpandIdents;

struct SavedFileInfo
{
	SavedFileInfo() :
		m_buffer(NULL)
	{
	}

	~SavedFileInfo()
	{
		delete [] m_buffer;
	}

	SimpleString m_fileName;
	unsigned char* m_buffer;
	size_t m_size;
};

typedef SimpleList<SavedFileInfo*> SavedFileInfoMap;
static SavedFileInfoMap m_savedFileInfoMap;

static void* GetAtSavedFileInfo(const char* key)
{
	for (void* pos = m_savedFileInfoMap.GetHeadPosition(); pos; m_savedFileInfoMap.GetNext(pos))
	{
		SavedFileInfo* info = m_savedFileInfoMap.GetAt(pos);
		if (info->m_fileName == key)
		{
			return pos;
		}
	}

	return NULL;
}


static void SetAtSavedFileInfo(SavedFileInfo* info)
{
	void* pos = GetAtSavedFileInfo(info->m_fileName);
	if (pos)
	{
		SavedFileInfo*& setAtInfo = m_savedFileInfoMap.GetAt(pos);
		setAtInfo = info;
	}
	else
	{
		m_savedFileInfoMap.AddTail(info);
	}
}


static LuaStateInfo* GetAtLuaStateInfo(LuaState* state)
{
	for (void* pos = luaStateInfoList.GetHeadPosition(); pos; luaStateInfoList.GetNext(pos))
	{
		LuaStateInfo& info = luaStateInfoList.GetAt(pos);
		if (info.state == state)
		{
			return &info;
		}
	}

	return NULL;
}


static LuaStateInfo* FindLuaStateInfo(const char* stateName)
{
	for (void* pos = luaStateInfoList.GetHeadPosition(); pos; luaStateInfoList.GetNext(pos))
	{
		LuaStateInfo& info = luaStateInfoList.GetAt(pos);
		if (info.stateName == stateName)
		{
			return &info;
		}
	}

	return NULL;
}


struct BreakpointInfo
{
	SimpleString m_fileName;
	int m_lineNumber;
};

typedef SimpleList<BreakpointInfo> BreakpointInfoArray;
static BreakpointInfoArray m_breakpointInfo;

enum RunType
{
	RUNTYPE_STEP_OVER,
	RUNTYPE_STEP_INTO,
	RUNTYPE_STEP_OUT,
	RUNTYPE_GO,
	RUNTYPE_BREAK,
};

static RunType m_runType = RUNTYPE_GO;

static SimpleString m_lastFileName;
static int m_lastLineNumber;

/**
	\internal
	Sends a network command to the Lua debugger.
**/
static void SendCommand(const char* command)
{
	m_networkServer->SendCommand( LUA_DEBUGGER_SOCKET_NAME, command );
}


/**
	\internal
	Binary writes a stored file across the network connection.

	\param fileName MUST be lowercase!!
**/
static void SendFileHelper(const char* fileName)
{
	// Get the socket for the Lua Debugger.  If it comes back NULL, then we
	// aren't connected.
	LuaNetworkSocket* socket = m_networkServer->GetSocket( LUA_DEBUGGER_SOCKET_NAME );
	if (socket)
	{
		// The socket is supposedly available, so look up the file info.
		SavedFileInfo* savedFileInfo = m_savedFileInfoMap.GetAt(GetAtSavedFileInfo(fileName));
		assert(savedFileInfo);

		// Send the file information to the Lua Debugger.
		socket->WriteBinary( savedFileInfo->m_buffer, savedFileInfo->m_size );
	}
}


/**
	Finds the index of the breakpoint matching [fileName] and [lineNumber].

	\param fileName MUST be lowercase.
	\param lineNumber
**/
static void* FindBreakpoint(const char* fileName, int lineNumber)
{
	// Iterate the breakpoint list.
	for (void* pos = m_breakpointInfo.GetHeadPosition(); pos; m_breakpointInfo.GetNext(pos))
	{
		// Is it a match?
		BreakpointInfo& info = m_breakpointInfo.GetAt(pos);
		if (info.m_fileName == fileName  &&  info.m_lineNumber == lineNumber)
			return pos;
	}

	return NULL;
}


static void luaI_addquotedbinary(SimpleString& str, const char* s, size_t l)
{
	str += "\"";
	while (l--)
	{
		switch (*s)
		{
			case '"':  case '\\':  case '\n':
			{
				str += "\\";
				str += SimpleString(s, 1);
				break;
			}
			default:
			{
				if (isprint((unsigned char)*s))
					str += *s;
				else
				{
					char buf[5];
					sprintf(buf, "\\%03d", (unsigned char)*s);
					str += buf;
				}
			}
		}
		s++;
	}
	str += "\"";
}


static void luaI_addquotedbinary (SimpleString& str, const char* s, int l)
{
	str += "\"";
	while (l--)
	{
		switch (*s)
		{
			case '"':  case '\\':
				str += "\\";
				str += SimpleString(s, 1);
				break;
			case '\a':		str += "\\a";		break;
			case '\b':		str += "\\b";		break;
			case '\f':		str += "\\f";		break;
			case '\n':		str += "\\n";		break;
			case '\r':		str += "\\r";		break;
			case '\t':		str += "\\t";		break;
			case '\v':		str += "\\v";		break;
			default:
				if (isprint((unsigned char)*s))
					str += *s;
				else
				{
					char buf[5];
					sprintf(buf, "\\x%02x", (unsigned long)(unsigned char)*s);
					str += buf;
				}
		}
		s++;
	}
	str += "\"";
}


#if LUA_WIDESTRING

static void luaI_addquotedwidebinary (SimpleString& str, const lua_WChar* s, int l)
{
	str += "L\"";
	while (l--)
	{
		switch (*s)
		{
			case '"':		str += "\\\"";		break;
			case '\\':		str += "\\\\";		break;
			case '\a':		str += "\\a";		break;
			case '\b':		str += "\\b";		break;
			case '\f':		str += "\\f";		break;
			case '\n':		str += "\\n";		break;
			case '\r':		str += "\\r";		break;
			case '\t':		str += "\\t";		break;
			case '\v':		str += "\\v";		break;
			default:
				if (*s < 256  &&  isprint((unsigned char)*s))
					str += (char)*s;
				else
				{
					char buf[20];
					sprintf(buf, "\\x%04x", (unsigned long)*s);
					str += buf;
				}
		}
		s++;
	}
	str += "\"";
}

#endif // LUA_WIDESTRING


/**
	Return a string representing the value portion of a key=value pair.
**/
static SimpleString DebugSerialize( LuaObject& valueObj )
{
	// Start off the output string.
	SimpleString str;

	// Grab the object type.
	int type = valueObj.Type();

	// If it is a number or string, then use the basic serialization approach.
	if ( type == LUA_TNUMBER )
	{
		char buf[500];
		sprintf(buf, "%.16g", valueObj.GetNumber());
		str += buf;
	}
	else if ( type == LUA_TSTRING )
		luaI_addquotedbinary( str, valueObj.GetString(), valueObj.StrLen() );
#if LUA_WIDESTRING
	else if ( type == LUA_TWSTRING )
		luaI_addquotedwidebinary( str, valueObj.GetWString(), valueObj.StrLen() );
#endif // LUA_WIDESTRING
	else if ( type == LUA_TBOOLEAN )
		str = valueObj.GetBoolean() ? "true" : "false";
	else if ( type == LUA_TTABLE )
		str = "'!!!TABLE!!!'";
	else if ( type == LUA_TFUNCTION )
	{
		if ( valueObj.IsCFunction() )
		    str = "'!!!CFUNCTION!!!'";
		else
			str = "'!!!FUNCTION!!!'";
	}
	else if (type == LUA_TNIL)
		str = "'!!!NIL!!!'";
	else if (type == LUA_TUSERDATA)
		str = "'!!!USERDATA!!!'";

	return str;
}


/**
	\internal
**/
static LuaObject GetValueObjectFromIdent( LuaState* luaStateToDebug, SimpleString ident )
{
	// Default to the globals table.
	LuaObject curTable = luaStateToDebug->GetGlobals();

	// Look for the first identifier:
	//   identifier1.identifier2.identifier3
	SimpleString findIdent = ident;
	int dotPos = findIdent.Find( '.' );
	if (dotPos != -1)
	{
		findIdent = findIdent.Left( dotPos );
	}

	// Walk through the various scopes.  Test for a local variable first.
	lua_Debug debugInfo;
	if ( luaStateToDebug->GetStack( 0, &debugInfo ) != 0 )
	{
		luaStateToDebug->GetInfo( "Sl", &debugInfo );

		// Wander the list of local variables.
		int i = 1;
		const char* localIdent;
		while ( ( localIdent = luaStateToDebug->GetLocal( &debugInfo, i++ ) ) != NULL )
		{
			// If the name matches the local variable, then choose it.
			if ( findIdent == localIdent )
			{
				// If we're dealing with a local variable that isn't a table
				// member, then do the assignment and leave.  Grab the local
				// variable value left on the stack from the traversal above.
				LuaStackObject localObj( luaStateToDebug, luaStateToDebug->GetTop() );

				// Were there any dotted members?
				if (dotPos == -1)
				{
					// No, just return the information we desire.
					return localObj;
				}

				// The local variable is a table.  Run the table object
				// recursively below.
				curTable = LuaStackObject( luaStateToDebug, luaStateToDebug->GetTop() );

				// Whack off the root object name.
				ident = ident.Mid( dotPos + 1 );

				break;
			}

			luaStateToDebug->Pop();  // remove variable value
		}

	}

	// Do a standard table traverse.  The table may be a local variable that
	// is a table or it may be the globals table.  At this point, we don't
	// care.
	while ( true )
	{
		// Search for the next period (should later extend to [] syntax, too).
		int dotPos = ident.Find( '.' );
		if (dotPos != -1)
		{
			// Yes, get the name of the object to find.
			findIdent = ident.Left( dotPos );

			// Strip off the beginning up to the dot.
			ident = ident.Mid( dotPos + 1 );
		}
		else
		{
			// No more tables?  Get outta here.
			findIdent = ident;
			break;
		}

		// Look up the identifier.
		LuaObject findIdentObj;
		int num = atoi( findIdent );
		if ( num != 0 )
			findIdentObj.AssignInteger( luaStateToDebug, num );
		else
			findIdentObj.AssignString( luaStateToDebug, findIdent );
		curTable = curTable[ findIdentObj ];
		if ( curTable.IsNil() )
			break;
	}

	// Retrieve the final object.
	LuaObject findIdentObj;
	int num = atoi( findIdent );
	if ( num != 0 )
		findIdentObj.AssignInteger( luaStateToDebug, num );
	else
		findIdentObj.AssignString( luaStateToDebug, findIdent );
	LuaObject obj = curTable.IsNil() ? curTable : ( findIdent.IsEmpty() ? curTable : curTable[ findIdentObj ] );

	// Return it.
	return obj;
}


struct KeyValueInfo
{
	SimpleString m_origKeyIdent;
	SimpleString m_key;
	SimpleString m_value;

	bool operator<( const KeyValueInfo& right )
	{
		return m_key < right.m_key;
	}
};

/**

**/
static void RecurseExpandIdents( LuaState* luaStateToDebug, ExpandIdentArray& expandIdents, SimpleString curParentIdent,
							LuaObject& curObj, SimpleString& watchStr, int indentLevel )
{
	if (expandIdents.GetCount() == 0)
		return;

	// Define the KeyValueList container.
	typedef SimpleList<KeyValueInfo> KeyValueList;
	KeyValueList keyValues;

	// Iterate the entire table.
	for ( LuaTableIterator it( curObj ); it; ++it )
	{
		LuaObject& keyObj = it.GetKey();
		LuaObject& valueObj = it.GetValue();

		KeyValueInfo keyValueInfo;
//		if ( keyObj.IsString() )
			keyValueInfo.m_origKeyIdent = DebugSerialize( keyObj );//.GetString();
		keyValueInfo.m_key = DebugSerialize( keyObj );
		keyValueInfo.m_value = DebugSerialize( valueObj );

		keyValues.AddTail( keyValueInfo );
	}

	keyValues.Sort();

	char buffer[20];
	sprintf(buffer, "%d", indentLevel);
	SimpleString indentLevelStr = buffer;

	curParentIdent += ".";

	for (void* pos = keyValues.GetHeadPosition(); pos; keyValues.GetNext(pos))
	{
		KeyValueInfo& info = keyValues.GetAt(pos);
		watchStr += "{";
		watchStr += info.m_key;
		watchStr += ",";
		watchStr += indentLevelStr;
		watchStr += ",";
		watchStr += info.m_value;
		watchStr += "}, ";

		if ( !info.m_origKeyIdent.IsEmpty() )
		{
			SimpleString curIdent = curParentIdent;
			SimpleString strippedOrigKeyIdent = info.m_origKeyIdent;
			if (strippedOrigKeyIdent[0] == '"')
				strippedOrigKeyIdent = strippedOrigKeyIdent.Mid(1);
			if (strippedOrigKeyIdent[strippedOrigKeyIdent.GetLength() - 1] == '"')
				strippedOrigKeyIdent = strippedOrigKeyIdent.Mid(0, strippedOrigKeyIdent.GetLength() - 1);
			curIdent += strippedOrigKeyIdent;

			for (void* expandPos = expandIdents.GetHeadPosition(); expandPos; expandIdents.GetNext(expandPos))
			{
				SimpleString& loopIdent = expandIdents.GetAt(expandPos);
				if ( curIdent == loopIdent )
				{
					LuaObject curIdentObj = GetValueObjectFromIdent( luaStateToDebug, curIdent );
					if ( !curIdentObj.IsNil() )
					{
						RecurseExpandIdents( luaStateToDebug, expandIdents, curIdent, curIdentObj, watchStr, indentLevel + 1 );
					}
					else
					{
						expandIdents.RemoveAt(expandPos);
					}

					break;
				}
			}
		}
	}
}


/**
	\internal
**/
static void SendLocalVariables(LuaState* luaStateToDebug)
{
	LuaStateInfo* luaStateInfo = GetAtLuaStateInfo(luaStateToDebug);
	if (!luaStateInfo)
		return;

	// Is there even a stack available?
	lua_Debug debugInfo;
	if ( luaStateToDebug->GetStack( 0, &debugInfo ) == 0 )
		return;

	// Yes, get information about it.
	luaStateToDebug->GetInfo( "Sl", &debugInfo );

	// Run the local variable list.
	int i = 1;
	const char* localIdent;
	SimpleString localsStr = "DebugUpdateLocals(\"" + luaStateInfo->stateName + "\",{";
	while ( ( localIdent = luaStateToDebug->GetLocal( &debugInfo, i++ ) ) != NULL )
	{
		if ( strcmp( localIdent, "(*temporary)" ) == 0 )
		{
			luaStateToDebug->Pop();  // remove variable value
			continue;
		}

		LuaObject localObj( luaStateToDebug, luaStateToDebug->GetTop() );
		SimpleString objStr = DebugSerialize( localObj );
		localsStr += "{'";
		localsStr += SimpleString( localIdent );
		localsStr += "',1,";
		localsStr += objStr;
		localsStr += "}, ";

		// Only expand identifiers if the root identifier is not nil.
		if ( localObj.IsTable() )
		{
			// If there are identifiers to be expanded within this current watch,
			// then call a special recursive function to handle it.
			RecurseExpandIdents( luaStateToDebug, m_localExpandIdents, localIdent, localObj, localsStr, 2 );
		}

		luaStateToDebug->Pop();  // remove variable value
	}

	localsStr += "})";

	SendCommand( localsStr );
}


/**
	Sends the entire list of watch variables to the Remote Lua Debugger.
**/
static void SendWatchVariables(LuaState* luaStateToDebug)
{
	LuaStateInfo* luaStateInfo = GetAtLuaStateInfo(luaStateToDebug);
	if (!luaStateInfo)
		return;

	// Wander the entire watch list.
	SimpleString watchStr = "DebugUpdateWatch(\"" + luaStateInfo->stateName + "\",{";
	for (void* pos = m_watchList.GetHeadPosition(); pos; m_watchList.GetNext(pos))
	{
		// Get the current watch name.
		WatchInfo& watchInfo = m_watchList.GetAt(pos);
		const SimpleString& ident = watchInfo.m_ident;

		LuaObject obj = GetValueObjectFromIdent( luaStateToDebug, ident );
		SimpleString objStr = DebugSerialize( obj );
		watchStr += "{'";
		watchStr += ident;
		watchStr += "',1,";
		watchStr += objStr;
		watchStr += "}, ";

		// Only expand identifiers if the root identifier is not nil.
		if ( obj.IsTable() )
		{
			// If there are identifiers to be expanded within this current watch,
			// then call a special recursive function to handle it.
			RecurseExpandIdents( luaStateToDebug, watchInfo.m_expandIdents, ident, obj, watchStr, 2 );
		}
	}

	watchStr += "})";

	// Finally, send the packet.
	SendCommand( watchStr );
}


/// From ldblib.c's errorfb.
#define LEVELS1	12	/* size of the first part of the stack */
#define LEVELS2	10	/* size of the second part of the stack */


/**
	Most of the code in this function was taken from ldblib.c's errorfb().
**/
static void SendCallStack(LuaState* luaStateToDebug)
{
	LuaStateInfo* luaStateInfo = GetAtLuaStateInfo(luaStateToDebug);
	if (!luaStateInfo)
		return;

	SimpleString out;
	int level = 0;
	int firstpart = 1;
	lua_Debug ar;

	out = "CallStack={";
	while ( luaStateToDebug->GetStack( level++, &ar ) )
	{
		if ( level > LEVELS1 && firstpart )
		{
			// no more than `LEVELS2' more levels?
			if ( !luaStateToDebug->GetStack( level+LEVELS2, &ar ) )
				level--;  // keep going
			else
			{
				// too many levels
				// find last levels
				while ( luaStateToDebug->GetStack( level+LEVELS2, &ar ) )
					level++;
			}
			firstpart = 0;
			continue;
		}
		out += "{";
		luaStateToDebug->GetInfo( "Snl", &ar );
		switch ( *ar.namewhat )
		{
			case 'g':  // global
			case 'l':  // local
			case 'f':  // field
			case 'm':  // method
				out += SimpleString( "[[" ) + ar.name + "]],'function'";
				break;

			default:
			{
				int start = 0;
				while (ar.short_src[start] != 0  &&  ar.short_src[start] == '[')
					start++;

				size_t end = strlen(ar.short_src);
				if (end > 0)
				{
					end--;
					while (end > start  &&  ar.short_src[end] == ']')
						end--;
				}

				SimpleString shortSrc(ar.short_src + start, end - start + 1);

				if ( *ar.what == 'm' )  // main?
					out += SimpleString("[[") + shortSrc + "]], 'main chunk'";
				else if (*ar.what == 'C')  // C function?
					out += SimpleString("[[") + shortSrc + "]],'c function'";
				else
				{
					SimpleString str;

					char lineDefined[20];
					sprintf(lineDefined, "%d", ar.linedefined);

					str = SimpleString("[[function <") + lineDefined + ":" + shortSrc + "]],";
					out += str + "'',";
				}
			 }
		}

		char currentline[20];
		sprintf(currentline, "%d", ar.currentline);
		out += "," + SimpleString(currentline) + "},\n";
	}
	out += "}";

	// Send the call stack to the debugger.
	SendCommand("DebugUpdateCallStack(\"" + luaStateInfo->stateName + "\")");
	m_networkServer->GetSocket(LUA_DEBUGGER_SOCKET_NAME)->WriteBinary((const char*)out, out.GetLength());
}


/**
**/
static void DebugLineHook( lua_State* inState, lua_Debug* /*ar*/ )
{
	LuaState* luaStateToDebug = lua_State_To_LuaState(inState);

	LuaStateInfo* luaStateInfo = GetAtLuaStateInfo(luaStateToDebug);
	if (!luaStateInfo)
		return;

	lua_Debug debugInfo;
	luaStateToDebug->GetStack( 0, &debugInfo );
	luaStateToDebug->GetInfo( "Sl", &debugInfo );

	if ( debugInfo.source[0] == '@' )
		debugInfo.source++;

	SimpleString curFileName = debugInfo.source;
	curFileName.MakeLower();
	curFileName.Replace('\\', '/');
	int curLineNumber = debugInfo.currentline - 1;

	if ( curLineNumber == m_lastLineNumber  &&  curFileName == m_lastFileName )
	{
		// No new line number.
		return;
	}

	curDebuggingState = luaStateToDebug;
	curDebuggingStateName = luaStateInfo->stateName;
	m_lastFileName = curFileName;
	m_lastLineNumber = curLineNumber;

	SimpleString justFileName;
	int slashPos = m_lastFileName.ReverseFind('/');
	if (slashPos == -1)
		justFileName = m_lastFileName;
	else
		justFileName = m_lastFileName.Mid(slashPos + 1);

	// If we're just running, then check for a breakpoint hit.
	if ( m_runType == RUNTYPE_GO  ||  m_runType == RUNTYPE_STEP_OUT  ||
		( m_runType == RUNTYPE_STEP_OVER  &&  m_nestedCalls > 0 ) )
	{
		// Wander the list looking for a matching breakpoint.
		if (FindBreakpoint( justFileName, m_lastLineNumber ) == NULL)
		{
			return;			// Didn't find a matching breakpoint.
		}

		// Got a match.
		m_runType = RUNTYPE_BREAK;
	}

	// Send the file name and line number to the remote debugger.
	char lineNumberStr[20];
	sprintf(lineNumberStr, "%d", m_lastLineNumber);

	SimpleString command = SimpleString("DebugSetLocation(\"") + luaStateInfo->stateName + "\", '" + justFileName + "', " + lineNumberStr + ")";
	SendCommand(command);

	// Send the local variables to the remote debugger.
	SendLocalVariables(luaStateToDebug);

	// Send the watch variables to the remote debugger.
	SendWatchVariables(luaStateToDebug);

	// Finally, send the call stack to the remote debugger.
	SendCallStack(luaStateToDebug);

	// Are we in a nested call for a step over?
	if (m_runType == RUNTYPE_STEP_OVER  &&  m_nestedCalls == 0)
	{
		// No, so just break.
		m_runType = RUNTYPE_BREAK;
	}

	// Sit idle until we receive a packet telling us what to do.
	while (true)
	{
		// Process network packets.
		LuaRemoteDebuggingServer::ProcessPackets();
		OsSleep( 1 );

		// If we were told to Step Over, then just exit.  Then next time
		// into the line hook, if the number of nested calls is 0, we can
		// just cause the run type to turn into a break (there was no
		// function to step over).
		if (m_runType == RUNTYPE_STEP_OVER)
		{
			break;
		}

		// If they accessed the Step Out command, leave Step Out as the
		// active run type and just exit the line hook.  The call hook
		// will keep track of nested calls since the Step Out command was
		// executed and cause a Break as soon as there are no more nested
		// calls.
		if (m_runType == RUNTYPE_STEP_OUT)
		{
			break;
		}

		// If we were told to go, then just leave.
		if (m_runType == RUNTYPE_GO)
		{
			break;
		}

		// If they accessed the Step Into command, then just break the run
		// for the next line.
		if (m_runType == RUNTYPE_STEP_INTO)
		{
			m_runType = RUNTYPE_BREAK;
			break;
		}
	}
}


/**
	\internal The internal Lua debug call hook.
**/
static void DebugCallHook( lua_State* /*inState*/, lua_Debug* /*inAr*/ )
{
	// See what run type we are.
	if ( m_runType == RUNTYPE_STEP_OVER  ||  m_runType == RUNTYPE_STEP_OUT )
	{
		// We're in step over/out mode.  That means for every function call we
		// receive, we expect an equal number of function returns.  When
		// the function returns are complete, then send the call stack.
		m_nestedCalls++;
	}
}


/**
	\internal The internal Lua debug return hook.
**/
static void DebugReturnHook( lua_State* /*inState*/, lua_Debug* /*inAr*/ )
{
	// See what run type we are.
	if ( m_runType == RUNTYPE_STEP_OVER  ||  m_runType == RUNTYPE_STEP_OUT )
	{
		// We're in step over/out mode.  That means for every function call we
		// receive, we expect an equal number of function returns.  When
		// the function returns are complete, then send the call stack.
		if ( m_nestedCalls == 0 )
		{
			m_runType = RUNTYPE_BREAK;
			m_nestedCalls = 0;
		}
		else
			m_nestedCalls--;
	}
}


/**
**/
static void DebugHook( lua_State* state, lua_Debug* ar )
{
	if ( ar->event == LUA_HOOKCALL )
		DebugCallHook( state, ar );
	else if ( ar->event == LUA_HOOKRET  ||  ar->event == LUA_HOOKTAILRET )
		DebugReturnHook( state, ar );
	else if ( ar->event == LUA_HOOKLINE )
		DebugLineHook( state, ar );
}


LuaState* FindLuaStateToDebug(LuaState* state)
{
	LuaStack args(state);
	const char* stateName = args[1].GetString();
	LuaStateInfo* info = FindLuaStateInfo(stateName);
	if (!info)
		return NULL;
	return info->state;
}


/**
	\internal
	LuaClientConnect()
**/
static int LS_LuaClientConnect( LuaState* state )
{
	// Push all the files over.
	void* pos;
	for (pos = m_savedFileInfoMap.GetHeadPosition(); pos; m_savedFileInfoMap.GetNext(pos))
	{
		SavedFileInfo* fileInfo = m_savedFileInfoMap.GetAt(pos);
		const SimpleString& fileName = fileInfo->m_fileName;

		SimpleString command = "DebugSendFile('" + fileName + "')";
		SendCommand( command );
		SendFileHelper( fileName );
	}

	for (pos = luaStateInfoList.GetHeadPosition(); pos; luaStateInfoList.GetNext(pos))
	{
		LuaStateInfo& info = luaStateInfoList.GetAt(pos);
		SendCommand("LuaDebuggerConnect(\"" + info.stateName + "\")");
	}

	// Set the current debug location.
	if ( !m_lastFileName.IsEmpty() )
	{
		char lineNumberStr[30];
		sprintf(lineNumberStr, "%d", m_lastLineNumber);

		SimpleString command = SimpleString("DebugSetLocation('") + curDebuggingStateName + "', '" + m_lastFileName + "', " + lineNumberStr + ")";
		SendCommand( command );
	}

	// Send the local variables to the remote debugger.
	SendLocalVariables(curDebuggingState);

	// Send the watch variables to the remote debugger.
	SendWatchVariables(curDebuggingState);

	// Finally, send the call stack to the remote debugger.
	SendCallStack(curDebuggingState);

	return 0;
}


/**
	\internal
	DebugOn()
**/
static int LS_DebugOn( LuaState* state )
{
	LuaState* luaStateToDebug = FindLuaStateToDebug(state);
	if (!luaStateToDebug)
		return 0;

	luaStateToDebug->SetHook( DebugHook, LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE, 0 );
	m_runType = RUNTYPE_BREAK;

	m_connected = true;

	return 0;
}


/**
	\internal
	DebugOff()
**/
static int LS_DebugOff( LuaState* state )
{
	LuaState* luaStateToDebug = FindLuaStateToDebug(state);
	if (!luaStateToDebug)
		return 0;

	luaStateToDebug->SetHook( NULL, LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE, 0 );

	return 0;
}


/**
	\internal
	DebugStepOver()
**/
static int LS_DebugStepOver( LuaState* state )
{
	if ( m_runType == RUNTYPE_BREAK )
	{
		m_runType = RUNTYPE_STEP_OVER;
	}
	return 0;
}


/**
	\internal
	DebugStepInto()
**/
static int LS_DebugStepInto( LuaState* state )
{
	m_runType = RUNTYPE_STEP_INTO;
	return 0;
}


/**
	\internal
	DebugGo()
**/
static int LS_DebugGo( LuaState* state )
{
	m_runType = RUNTYPE_GO;
	return 0;
}


/**
	\internal
	DebugBreak()
**/
static int LS_DebugBreak( LuaState* state )
{
	m_runType = RUNTYPE_BREAK;
	return 0;
}


/**
	\internal
	DebugStepOut()
**/
static int LS_DebugStepOut( LuaState* state )
{
	if ( m_runType == RUNTYPE_BREAK )
	{
		m_runType = RUNTYPE_STEP_OUT;
	}
	return 0;
}


/**
	\internal
	DebugSetBreakpoint( fileName, lineNumber, on )
**/
static int LS_DebugSetBreakpoint( LuaState* state )
{
	// Make the file name lowercase and only handle forward slashes.
	LuaStack args(state);
	SimpleString fileName = args[ 1 ].GetString();
	fileName.MakeLower();
	fileName.Replace( '\\', '/' );

	int lineNumber = args[ 2 ].GetInteger();
	bool on = args[ 3 ].GetBoolean();

	// See if the breakpoint already exists.
	void* breakpointIndex = FindBreakpoint( fileName, lineNumber );
	if ( on )
	{
		// Did we find the breakpoint in the list?
		if (breakpointIndex == NULL)
		{
			// Add the breakpoint.
			BreakpointInfo info;
			info.m_fileName = fileName;
			info.m_lineNumber = lineNumber;
			m_breakpointInfo.AddTail(info);
		}
	}
	else
	{
		// Did we find the breakpoint?
		if (breakpointIndex != NULL)
		{
			// Yes.  Remove it, since we're turning it off.
			m_breakpointInfo.RemoveAt(breakpointIndex);
		}
	}

	return 0;
}


/**
	Pushes a value onto the Lua state to debug.  Only handles numbers, strings,
	and nil right now.
**/
void PushDebugObject( LuaState* luaStateToDebug, LuaStackObject valueObj )
{
	if ( valueObj.IsNumber() )
	{
		luaStateToDebug->PushNumber( valueObj.GetNumber() );
	}
	else if ( valueObj.IsString() )
	{
		luaStateToDebug->PushString( valueObj.GetString() );
	}
	else if ( valueObj.IsNil() )
	{
		luaStateToDebug->PushNil();
	}
}


/**
	\internal
	DebugSetVariable( name, value )

	\todo Handle array cases.  Only handles strings right now.
**/
static int LS_DebugSetVariable( LuaState* state )
{
	LuaState* luaStateToDebug = FindLuaStateToDebug(state);
	if (!luaStateToDebug)
		return 0;

	LuaStack args(state);
	SimpleString ident = args[2].GetString();
	LuaStackObject valueObj = args[3];
	SimpleString origIdent = ident;

	// Default to the globals table.
	LuaObject curTable = luaStateToDebug->GetGlobals();

	// Look for the first identifier:
	//   identifier1.identifier2.identifier3
	SimpleString findIdent = ident;
	int dotPos = findIdent.Find( '.' );
	if (dotPos != -1)
		findIdent = findIdent.Left( dotPos );

	// Test for a local variable.
	lua_Debug debugInfo;
	luaStateToDebug->GetStack( 0, &debugInfo );
	luaStateToDebug->GetInfo( "Sl", &debugInfo );

	// Iterate the local variables list.
	int i = 1;
	const char* localIdent;
	while ( ( localIdent = luaStateToDebug->GetLocal( &debugInfo, i++ ) ) != NULL )
	{
		// Does it match one of the local variable names?
		if ( findIdent == localIdent )
		{
			// If we're dealing with a local variable that isn't a table member, then
			// do the assignment and leave.
			if (dotPos == -1)
			{
				// Set the local variable.
				PushDebugObject( luaStateToDebug, valueObj );
				luaStateToDebug->SetLocal( &debugInfo, i - 1 );
				return 0;
			}

			// The local variable is a table.
			curTable = LuaStackObject( luaStateToDebug, luaStateToDebug->GetTop() );
			ident = ident.Mid( dotPos + 1 );
			break;
		}

		// Remove variable value to iterate for the next local.
		luaStateToDebug->Pop();
	}

	// Do a standard table traverse.  The table may be a local variable that
	// is a table or it may be the globals table.  At this point, we don't
	// care.
	while ( true )
	{
		// Search for the next period.
		int dotPos = ident.Find( '.' );
		if (dotPos != -1)
		{
			findIdent = ident.Left( dotPos );
			ident = ident.Mid( dotPos + 1 );
		}
		else
		{
			// No more tables?  Get outta here.
			findIdent = ident;
			break;
		}

		// Look up the identifier.
		LuaObject obj = curTable[ findIdent ];
		if ( !obj.IsTable() )
		{
			// Automatically create the table for the user if it doesn't exist.
			obj = curTable.CreateTable( findIdent );
		}

		curTable = obj;
	}

	// Finally, set the value.
    curTable.Push();
	LuaStackObject curTableStackObj = luaStateToDebug->StackTop();
	luaStateToDebug->PushString( findIdent );
	PushDebugObject( luaStateToDebug, valueObj );
	luaStateToDebug->SetTable(curTableStackObj);
	luaStateToDebug->Pop();

	return 0;
}


/**
	\internal
	DebugReadWatchVariables( name, index )
**/
static int LS_DebugReadWatchVariables( LuaState* state )
{
	LuaState* luaStateToDebug = FindLuaStateToDebug(state);
	if (!luaStateToDebug)
		return 0;
	SendWatchVariables(luaStateToDebug);

	return 0;
}


/**
	\internal
	DebugReadLocalVariables( name, index )
**/
static int LS_DebugReadLocalVariables( LuaState* state )
{
	LuaState* luaStateToDebug = FindLuaStateToDebug(state);
	if (!luaStateToDebug)
		return 0;
	SendLocalVariables(luaStateToDebug);

	return 0;
}


/**
	\internal
	DebugAddWatch( index, ident )
**/
static int LS_DebugAddWatch( LuaState* state )
{
	LuaState* luaStateToDebug = FindLuaStateToDebug(state);
	if (!luaStateToDebug)
		return 0;

	LuaStack args(state);
	int index = args[2].GetInteger();
	SimpleString ident = args[3].GetString();

	if (m_watchList.GetCount() <= (size_t)index)
	{
		WatchInfo& watchInfo = m_watchList.GetAt(m_watchList.AddTail());
		watchInfo.m_ident = ident;
	}
	else
	{
		int loopIndex = 0;
		for (void* pos = m_watchList.GetHeadPosition(); pos; m_watchList.GetNext(pos), ++loopIndex)
		{
			if (loopIndex++ == index)
			{
				// Get the current watch name.
				WatchInfo& watchInfo = m_watchList.GetAt(pos);
				watchInfo.m_ident = ident;
				break;
			}
		}
	}

	return 0;
}


/**
	\internal
	DebugRemoveWatch( index )
**/
static int LS_DebugRemoveWatch( LuaState* state )
{
	LuaState* luaStateToDebug = FindLuaStateToDebug(state);
	if (!luaStateToDebug)
		return 0;

	LuaStack args(state);
	int index = args[2].GetInteger();

	int loopIndex = 0;
	for (void* pos = m_watchList.GetHeadPosition(); pos; m_watchList.GetNext(pos), ++loopIndex)
	{
		if (loopIndex++ == index)
		{
			m_watchList.RemoveAt(pos);
			break;
		}
	}

	return 0;
}


/**
	\internal
**/
static void AddExpandIdentHelper( ExpandIdentArray& expandIdents, SimpleString& expandIdent )
{
	// Wander the expand info to find an expand identifier matching the
	// requested one.
	for (void* pos = expandIdents.GetHeadPosition(); pos; expandIdents.GetNext(pos))
	{
		if (expandIdents.GetAt(pos) == expandIdent)
			return;
	}

	// Didn't find it.  Make a new one.
	expandIdents.AddTail(expandIdent);
}


/**
	\internal
**/
static void RemoveExpandIdentHelper( ExpandIdentArray& expandIdents, const SimpleString& expandIdent )
{
	// Wander the watch expand info to find an expand identifier matching the
	// requested one.
	for (void* pos = expandIdents.GetHeadPosition(); pos;)
	{
		void* oldPos = pos;
		SimpleString& curIdent = expandIdents.GetNext(pos);
		if ( curIdent.Left( expandIdent.GetLength() ) == expandIdent )
		{
			expandIdents.RemoveAt(oldPos);
		}
	}
}


/**
	\internal
	DebugAddWatchExpandIdent( index, expandIdent )
**/
static int LS_DebugAddWatchExpandIdent( LuaState* state )
{
	LuaState* luaStateToDebug = FindLuaStateToDebug(state);
	if (!luaStateToDebug)
		return 0;

	LuaStack args(state);
	int index = args[2].GetInteger();
	SimpleString expandIdent = args[3].GetString();

	// Get the proper watch info structure.
	int loopIndex = 0;
	for (void* pos = m_watchList.GetHeadPosition(); pos; m_watchList.GetNext(pos))
	{
		if (loopIndex++ == index)
		{
			WatchInfo& watchInfo = m_watchList.GetAt(pos);
			AddExpandIdentHelper( watchInfo.m_expandIdents, expandIdent );

			SendWatchVariables(luaStateToDebug);
		}
	}

	return 0;
}


/**
	\internal
	DebugRemoveWatchExpandIdent( index, expandIdent )
**/
static int LS_DebugRemoveWatchExpandIdent( LuaState* state )
{
	LuaState* luaStateToDebug = FindLuaStateToDebug(state);
	if (!luaStateToDebug)
		return 0;

	LuaStack args(state);
	int index = args[2].GetInteger();
	SimpleString expandIdent = args[3].GetString();

	// Get the proper watch info structure.
	int loopIndex = 0;
	for (void* pos = m_watchList.GetHeadPosition(); pos; m_watchList.GetNext(pos))
	{
		if (loopIndex++ == index)
		{
			WatchInfo& watchInfo = m_watchList.GetAt(pos);
			RemoveExpandIdentHelper( watchInfo.m_expandIdents, expandIdent );

			SendWatchVariables(luaStateToDebug);
		}
	}

	return 0;
}


/**
	\internal
	DebugAddLocalExpandIdent( expandIdent )
**/
static int LS_DebugAddLocalExpandIdent( LuaState* state )
{
	LuaState* luaStateToDebug = FindLuaStateToDebug(state);
	if (!luaStateToDebug)
		return 0;

	LuaStack args(state);
	SimpleString expandIdent = args[3].GetString();
	AddExpandIdentHelper( m_localExpandIdents, expandIdent );
	SendLocalVariables(luaStateToDebug);

	return 0;
}


/**
	\internal
	DebugRemoveLocalExpandIdent( index, expandIdent )
**/
static int LS_DebugRemoveLocalExpandIdent( LuaState* state )
{
	LuaState* luaStateToDebug = FindLuaStateToDebug(state);
	if (!luaStateToDebug)
		return 0;

	LuaStack args(state);
	SimpleString expandIdent = args[3].GetString();
	RemoveExpandIdentHelper( m_localExpandIdents, expandIdent );
	SendLocalVariables(luaStateToDebug);

	return 0;
}


/**
	\internal
	LS_DebugOutput()
**/
static int LS_DebugOutput( LuaState* state )
{
	LuaState* luaStateToDebug = FindLuaStateToDebug(state);
	if (!luaStateToDebug)
		return 0;

	LuaStack args(state);
	LuaStackObject identObj = args[2];
	LuaRemoteDebuggingServer::DebugOutput( identObj.GetString(), identObj.StrLen() );
	return 0;
}


/**
	\internal
	LS_DebugDoBuffer()
	receives a buffer for the luaStateToDebug to
**/
static int LS_DebugDoBuffer( LuaState* state )
{
	LuaState* luaStateToDebug = FindLuaStateToDebug(state);
	if (!luaStateToDebug)
		return 0;

	LuaStack args(state);
	LuaStackObject bufferToSend = args[2];
	luaL_loadbuffer( *luaStateToDebug, bufferToSend.GetString(), bufferToSend.StrLen(), LUA_DEBUGGER_SOCKET_NAME ) || lua_pcall(*luaStateToDebug, 0, 0, 0);
	return 0;
}


/**
	Initializes the debugging server.  Currently, the server only accepts one
	Lua state.

	\param luaStateToDebug The pointer to the previously opened lua_State
		the server should debug.
	\param networkConnectionFileName Provides the opportunity to specify the
		location of the file whereupon network connection requests are stored.
		If the server is shut down, it will attempt to reconnect with the
		last debugger connection when it starts back up.  If NULL, a reasonable
		default is provided.
**/
void LuaRemoteDebuggingServer::Initialize( const char* networkConnectionFileName )
{
	// Initialize the network server.
	m_networkServer = LuaNetworkServer::CreateInstance();

	// If no network connection file name was passed in, assign a default.
	if ( !networkConnectionFileName )
	{
		networkConnectionFileName = "c:\\RemoteLuaDebuggingServerNetworkConnection.txt";
	}

	// Make the Lua state we'll use for network communications.
	m_networkLuaState = lua_State_To_LuaState(lua_open());

	// Register all the callback functions.
	LuaObject globalsObj = m_networkLuaState->GetGlobals();
	globalsObj.Register("DebugOn", LS_DebugOn);
	globalsObj.Register("DebugOff", LS_DebugOff);
	globalsObj.Register("DebugSetBreakpoint", LS_DebugSetBreakpoint);
	globalsObj.Register("DebugStepOver", LS_DebugStepOver);
	globalsObj.Register("DebugStepInto", LS_DebugStepInto);
	globalsObj.Register("DebugStepOut", LS_DebugStepOut);
	globalsObj.Register("DebugGo", LS_DebugGo);
	globalsObj.Register("DebugBreak", LS_DebugBreak);
	globalsObj.Register("DebugSetVariable", LS_DebugSetVariable);
	globalsObj.Register("DebugReadWatchVariables", LS_DebugReadWatchVariables);
	globalsObj.Register("DebugReadLocalVariables", LS_DebugReadLocalVariables);
	globalsObj.Register("DebugAddWatch", LS_DebugAddWatch);
	globalsObj.Register("DebugRemoveWatch", LS_DebugRemoveWatch);
	globalsObj.Register("DebugAddLocalExpandIdent", LS_DebugAddLocalExpandIdent);
	globalsObj.Register("DebugRemoveLocalExpandIdent", LS_DebugRemoveLocalExpandIdent);
	globalsObj.Register("DebugAddWatchExpandIdent", LS_DebugAddWatchExpandIdent);
	globalsObj.Register("DebugRemoveWatchExpandIdent", LS_DebugRemoveWatchExpandIdent);
	globalsObj.Register("LuaClientConnect", LS_LuaClientConnect);
	globalsObj.Register("DebugOutput", LS_DebugOutput);
	globalsObj.Register("DebugDoBuffer", LS_DebugDoBuffer);

	// Register it with the network server, which will expect to communicate
	// with the state via network messages that start with LuaDebugger.
	m_networkServer->RegisterID( LUA_DEBUGGER_SOCKET_NAME, m_networkLuaState->GetCState(), LuaNetworkServer::LuaPlus_ProcessCommand<void> );

	// Open the network server.
	m_networkServer->Open( networkConnectionFileName, LUA_DEBUGGER_SERVER_PORT );
}


static void LoadNotifyFunction(lua_State *L, const char *name)
{
	if (name[0] == '@')
		++name;

	FILE* file = fopen( name, "rb" );
	if ( !file )
		return;

	long currentPos = ftell( file );
	fseek( file, 0, SEEK_END );
	long size = ftell( file );
	fseek( file, currentPos, SEEK_SET );

	unsigned char* buf = (unsigned char*)malloc( size + 1 );
	fread( buf, size, 1, file );
	fclose( file );

	buf[size] = 0;

	LuaRemoteDebuggingServer::AttachScriptFile(name, buf, size);

	free(buf);
}


void LuaRemoteDebuggingServer::RegisterState(const char* stateName, lua_State* L)
{
	LuaState* state = lua_State_To_LuaState(L);
	LuaStateInfo* info = FindLuaStateInfo(stateName);
	if (info)
	{
		info->state = state;
	}
	else
	{
		LuaStateInfo newInfo;
		newInfo.state = state;
		newInfo.stateName = stateName;
		luaStateInfoList.AddTail(newInfo);
	}

	lua_setloadnotifyfunction(L, LoadNotifyFunction);
}


/**
	Shuts down the debugging server.
**/
void LuaRemoteDebuggingServer::Shutdown()
{
	// Close the network server.
	LuaNetworkServer::DestroyInstance(m_networkServer);

	if (m_networkLuaState)
	{
		lua_close( LuaState_to_lua_State( m_networkLuaState ) );
		m_networkLuaState = NULL;
	}

	// Clear out all the saved file information.
	for (void* pos = m_savedFileInfoMap.GetHeadPosition(); pos; m_savedFileInfoMap.GetNext(pos))
	{
		delete m_savedFileInfoMap.GetAt(pos);
	}

	m_savedFileInfoMap.RemoveAll();
}


/**
	Wait indefinitely for a debugger connection.
**/
void LuaRemoteDebuggingServer::WaitForDebuggerConnection()
{
	while ( !m_connected )
	{
		ProcessPackets();
		OsSleep(50);
	}

	m_runType = RUNTYPE_BREAK;
}


/**
	AttachScriptFile() inserts a file into an internal buffer so the
	LuaRemoteDebuggingServer can pass the file information to the actual
	debugger client.

	\param srcFileName
	\param srcBuffer
	\param srcBufferSize
**/
void LuaRemoteDebuggingServer::AttachScriptFile( const char* srcFileName,
												unsigned char* srcBuffer,
												size_t srcBufferSize )
{
	// Store off the file name in our own format.
	SimpleString fileName = srcFileName;

	unsigned char* buffer = new unsigned char[ srcBufferSize ];
	memcpy( buffer, srcBuffer, srcBufferSize );

	// Make it lowercase for the map lookup.  Also, deal only in forward
	// slashes.
	fileName.MakeLower();
	fileName.Replace( '\\', '/' );

	// Try looking up the file name in the map.
	void* pos = GetAtSavedFileInfo(fileName);

	// Does it already exist?
	if (!pos)
	{
		// No.  Make a new structure and add it to the map.
		SavedFileInfo* savedFileInfo = new SavedFileInfo;
		savedFileInfo->m_fileName = fileName;
		savedFileInfo->m_buffer = buffer;
		savedFileInfo->m_size = srcBufferSize;

		SetAtSavedFileInfo(savedFileInfo);
	}
	else
	{
		// Yes.  Replace the contents of the structure.
		SavedFileInfo* savedFileInfo = m_savedFileInfoMap.GetAt(pos);
		delete[] savedFileInfo->m_buffer;
		savedFileInfo->m_buffer = buffer;
		savedFileInfo->m_size = srcBufferSize;
	}

	// Send the file and its contents to the debugger.
	SimpleString command = "DebugSendFile('" + fileName + "')";
	SendCommand( command );
	SendFileHelper( fileName );
}


/**
	Process any network packets.
**/
void LuaRemoteDebuggingServer::ProcessPackets()
{
	if (m_networkServer)
		m_networkServer->ProcessPackets();
}


/**
	Sends text to output to the debugger output window.
**/
void LuaRemoteDebuggingServer::DebugOutput( const char* srcBuffer, size_t srcBufferSize )
{
	SimpleString cleanluaString;
	luaI_addquotedbinary( cleanluaString, srcBuffer, srcBufferSize );

	SimpleString command = "DebugOutput(" + cleanluaString + ")";
	SendCommand(command);
}

/**
	Returns true if the server is connected to a client.
**/
bool LuaRemoteDebuggingServer::IsConnected()
{
	return m_networkServer != NULL;
}

