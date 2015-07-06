#pragma once

namespace LuaDebugger {

class LuaNetworkSocket;

class LuaNetworkServer
{
public:
	template<typename T>
	static void LuaPlus_ProcessCommand(const char* /*id*/, void* userData, const unsigned char* command)
	{
		lua_State* state = (lua_State*)userData;
#if LUA_WIDESTRING_FILE
		if (*(unsigned short*)command == 0xfffe)
			luaL_dowstring(state, (const lua_WChar*)(command + 2));
		else
#endif // LUA_WIDESTRING
			luaL_dostring(state, (const char*)command);
	}

	typedef void (*fnProcessCommand)(const char* id, void* userData, const unsigned char* command);

	static LuaNetworkServer* CreateInstance();
	static void DestroyInstance(LuaNetworkServer* server);

	virtual bool Open( const char* serverInfoFileName, unsigned long serverPort = 5001 ) = 0;
	virtual void Close() = 0;

	virtual LuaNetworkSocket* GetSocket( const char* id ) = 0;
	virtual void SendCommand( const char* id, const char* command ) = 0;
	virtual void ProcessPackets() = 0;

	virtual void RegisterID( const char* id, void* userData, fnProcessCommand processCommand ) = 0;
	virtual void* GetIDUserData( const char* id ) = 0;

protected:
	LuaNetworkServer() {};
	virtual ~LuaNetworkServer() {};
};

} // namespace LuaDebugger
