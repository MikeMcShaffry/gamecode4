#pragma once

#include <windows.h>
#include "LuaNetworkHelpers.h"

struct lua_State;

namespace LuaDebugger {

class LuaNetworkClient
{
public:
	// Constructors
	LuaNetworkClient();
	virtual ~LuaNetworkClient();

	bool Open(const char* id, const char* registryKey, lua_State* state,
			SockAddr* sockAddr, DWORD serverPort = 5001);
	void Close();

	// Attributes
	LuaNetworkSocket& GetSocket()			{  return m_socket;  }

	// Operations
	void ProcessPackets();
	void SendCommand(LPCSTR command);
	void SendCommand(LPCWSTR command);

	void Connect();

	virtual void OnConnect()		{}

protected:
	friend class LuaClientListenerThread;
	friend class AutoConnectThread;

	DWORD ReadPortFromRegistry();
	void WritePortToRegistry();
	
	SockAddr m_connectSockAddr;
	AutoConnectThread* m_autoConnectThread;

	SimpleString m_id;
	SimpleString m_registryKey;
	LuaNetworkSocket m_socket;
	SockAddr m_socketSockAddr;
	DWORD m_serverPort;
	LuaClientListenerThread* m_listenerThread;
	CRITICAL_SECTION m_socketAccessCS;
	lua_State* m_state;
};

} // namespace LuaDebugger