/**
	\file The header file of the LuaRemoteDebuggingServer interface.
**/
#ifndef LUAREMOTEDEBUGGINGSERVER_H
#define LUAREMOTEDEBUGGINGSERVER_H

struct lua_State;

const unsigned long LUA_DEBUGGER_SERVER_PORT = 3536;

/**
**/
class LuaRemoteDebuggingServer
{
public:
	static void Initialize( const char* networkConnectionFileName = 0 );
	static void RegisterState( const char* stateName, lua_State* state );
	static void Shutdown();
	static void WaitForDebuggerConnection();
	static void AttachScriptFile( const char* srcFileName, unsigned char* srcBuffer,
			size_t srcBufferSize );
	static void ProcessPackets();
	static void DebugOutput( const char* srcBuffer, size_t srcBufferSize );
	static bool IsConnected();
};

#endif // LUAREMOTEDEBUGGINGSERVER_H
