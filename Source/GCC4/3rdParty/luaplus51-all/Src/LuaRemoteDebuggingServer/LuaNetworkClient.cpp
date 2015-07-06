#include "LuaNetworkClient.h"
#include <stdio.h>
#include "LuaPlus/LuaPlus.h"
#include <process.h>

#define ULTRA_DEBUG

namespace LuaDebugger {

class LuaNetworkThread
{
public:
	LuaNetworkThread() :
		m_threadKillEvent(NULL),
		m_threadHandle(NULL)
	{
	}
	
	virtual ~LuaNetworkThread()
	{
		ThreadDestroy();
	}

	void ThreadCreate()
	{
		m_threadKillEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
		m_threadHandle = (HANDLE)_beginthreadex( 0, 0, WorkerThread, this, 0, NULL );
	}

	void ThreadDestroy(bool wait = true)
	{
		if ( !m_threadHandle )
			return;

		::SetEvent( m_threadKillEvent );

		if ( wait )
			WaitForSingleObject( m_threadHandle, INFINITE );

		::CloseHandle( m_threadHandle );
		::CloseHandle( m_threadKillEvent );

		m_threadKillEvent	= NULL;
		m_threadHandle		= NULL;
	}
	
	bool ThreadExitTriggered()
	{
		return (WaitForSingleObject(m_threadKillEvent, 0) == WAIT_OBJECT_0);
	}

	virtual void ThreadRun() = 0;

private:
	static UINT __stdcall WorkerThread(void* context);

	volatile HANDLE m_threadKillEvent;
	volatile HANDLE m_threadHandle;
};

inline UINT __stdcall LuaNetworkThread::WorkerThread(void* context)
{
	LuaNetworkThread* thread = (LuaNetworkThread*)context;
	thread->ThreadRun();

	_endthreadex(0);

	return 0;
}

const int LUA_DEBUGGER_PORT = 3535;

const DWORD MAGIC_NUMBER = 0xFEDC;

struct SocketConnectPacket
{
	DWORD magicNumber;
	DWORD port;
	char id[32];
};

class LuaClientListenerThread : public LuaNetworkThread
{
public:
	LuaClientListenerThread(LuaNetworkClient* parent) :
		m_parent(parent)
	{
		m_socketCreatedEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	}

	~LuaClientListenerThread()
	{
		CloseHandle( m_socketCreatedEvent );
	}

	virtual void ThreadRun()
	{
		DWORD port = m_parent->ReadPortFromRegistry();
		if ( port == 0 )
			port = LUA_DEBUGGER_PORT;

		LuaNetworkSocket socket;
		if (!socket.Create(port))		// Auto pick a port if 0.
		{
			socket.Create(0);			// Force auto pick.
		}

		m_parent->m_socketSockAddr = socket.GetSockName();
		m_parent->m_socketSockAddr.GetPort();
		m_parent->WritePortToRegistry();

		SetEvent( m_socketCreatedEvent );

		while (!ThreadExitTriggered())
		{
			if (!socket.IsReadDataWaiting())
			{
				Sleep(50);
				continue;
			}

			SockAddr sockAddr;
			LuaNetworkSocket inSocket = socket.Accept(&sockAddr);
			if (!inSocket.IsValid())
			{
				Sleep(50);
				continue;
			}

#ifdef ULTRA_DEBUG			
			{
				SockAddr::IPString ipAddr;
				sockAddr.GetIPAndPortString(ipAddr);
				SimpleString str = SimpleString("LuaNetworkClient: Incoming connection request from [") + ipAddr + "]\n";
				OutputDebugStringA( str );
			}
#endif ULTRA_DEBUG

			EnterCriticalSection( &m_parent->m_socketAccessCS );

			// The client allows only one server connection at a time.
			m_parent->m_socket.Close();
			m_parent->m_socket.Attach(inSocket);
			inSocket.Attach(INVALID_SOCKET);

			DWORD startTime = GetTickCount();
			while ( GetTickCount() - startTime < 3000 )
			{
				DWORD count = m_parent->m_socket.GetReadDataWaiting();
				if ( count < 4 )
				{
					Sleep(50);
					continue;
				}

				char id[33];
				memset(id, 0, sizeof(id));
				m_parent->m_socket.ReadString(id, 32);

				if (id == m_parent->m_id)
				{
					m_parent->OnConnect();
					break;
				}

				m_parent->m_socket.Close();
				break;
			}

			LeaveCriticalSection( &m_parent->m_socketAccessCS );

			Sleep(50);
		}
	}
	
	HANDLE m_socketCreatedEvent;

protected:
	LuaNetworkClient* m_parent;
};


class AutoConnectThread : public LuaNetworkThread
{
public:
	AutoConnectThread(LuaNetworkClient* parent) :
		m_parent(parent)
	{
	}

	virtual void ThreadRun()
	{
		while (!ThreadExitTriggered())
		{
			m_parent->Connect();
			Sleep(5000);
		}
	}
	
protected:
	LuaNetworkClient* m_parent;
};


/**
**/
LuaNetworkClient::LuaNetworkClient() :
	m_listenerThread(NULL),
	m_autoConnectThread(NULL),
	m_state(NULL),
	m_serverPort(5001)
{
	InitializeCriticalSection( &m_socketAccessCS );
}


/**
**/
LuaNetworkClient::~LuaNetworkClient()
{
	DeleteCriticalSection( &m_socketAccessCS );
}

/**
**/
bool LuaNetworkClient::Open(const char* id, const char* registryKey, 
					   lua_State* state, SockAddr* outSockAddr, DWORD serverPort)
{
	m_registryKey = SimpleString("Software\\") + registryKey;
	m_state = state;
	m_id = id;
	m_serverPort = serverPort;

	m_listenerThread = new LuaClientListenerThread(this);
	m_listenerThread->ThreadCreate();
	WaitForSingleObject( ((LuaClientListenerThread*)m_listenerThread)->m_socketCreatedEvent, INFINITE );

	SockAddr sockAddr;
	if (outSockAddr)
		sockAddr = *outSockAddr;
	else
		sockAddr = SockAddr("127.0.0.1", 0);
	m_connectSockAddr = sockAddr;

	m_autoConnectThread = new AutoConnectThread(this);
	m_autoConnectThread->ThreadCreate();
	
	return true;
}


void LuaNetworkClient::Close()
{
	delete m_listenerThread;
	m_listenerThread = NULL;

	delete m_autoConnectThread;
	m_autoConnectThread = NULL;
}


void LuaNetworkClient::Connect()
{
	if (m_socket.IsValid())
		return;

	EnterCriticalSection( &m_socketAccessCS );

	LuaNetworkSocket socket;
	m_connectSockAddr.SetPort(m_serverPort);
	if (!socket.Connect(m_connectSockAddr))
	{
		LeaveCriticalSection( &m_socketAccessCS );
		return;
	}

	SocketConnectPacket connectPacket;
	connectPacket.magicNumber = htonl(MAGIC_NUMBER);
	connectPacket.port = htonl(m_socketSockAddr.GetPort());
	strcpy(connectPacket.id, m_id);
	socket.Write(&connectPacket, sizeof(SocketConnectPacket));
	socket.Close();

	LeaveCriticalSection( &m_socketAccessCS );
}


void LuaNetworkClient::ProcessPackets()
{
	if (!m_socket.IsValid())
		return;

	EnterCriticalSection( &m_socketAccessCS );

	while (m_socket.IsReadDataWaiting())
	{
		SimpleString command;
		if (ReadString(m_socket, command))
		{
			luaL_dostring(m_state, command);
		}
	}

	LeaveCriticalSection( &m_socketAccessCS );
}


void LuaNetworkClient::SendCommand(LPCSTR command)
{
	EnterCriticalSection( &m_socketAccessCS );
	
	if (m_socket.IsValid())
		m_socket.WriteString(command);

	LeaveCriticalSection( &m_socketAccessCS );
}


void LuaNetworkClient::SendCommand(LPCWSTR command)
{
	EnterCriticalSection( &m_socketAccessCS );
	
	if (m_socket.IsValid())
		m_socket.WriteString(command);

	LeaveCriticalSection( &m_socketAccessCS );
}


#pragma comment(lib, "advapi32.lib")

/**
**/
DWORD LuaNetworkClient::ReadPortFromRegistry()
{
	// Get the port name.
	HKEY regKey = NULL;
	
	if (RegOpenKeyExA(HKEY_CURRENT_USER, m_registryKey, 0, KEY_ALL_ACCESS, &regKey) == ERROR_SUCCESS)
	{
		DWORD value;
		DWORD valueLen = sizeof(DWORD);
		DWORD dwType = NULL;
		if (RegQueryValueExA(regKey, (m_id + "_Port"), NULL, &dwType,
			(LPBYTE)&value, &valueLen) == ERROR_SUCCESS)
		{
			return value;
		}

		RegCloseKey(regKey);
	}

	return 0;
}


/**
**/
void LuaNetworkClient::WritePortToRegistry()
{
	HKEY regKey = NULL;
	
	if (RegCreateKeyExA(HKEY_CURRENT_USER, m_registryKey, NULL, NULL, 0,
			KEY_ALL_ACCESS, NULL, &regKey, NULL) == ERROR_SUCCESS)
	{
		DWORD port = m_socketSockAddr.GetPort();
		RegSetValueExA(regKey, m_id + "_Port", NULL, REG_DWORD, (BYTE * const)&port, sizeof(DWORD));

		RegCloseKey(regKey);
	}
}

} // namespace LuaDebugger
