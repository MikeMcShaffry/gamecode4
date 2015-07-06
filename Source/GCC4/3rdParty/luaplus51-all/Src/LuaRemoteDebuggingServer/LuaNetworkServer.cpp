#include <stdio.h>
#include <time.h>
#include <winsock2.h>
#undef SetPort
#include "LuaNetworkHelpers.h"
#include "LuaNetworkServer.h"

#define ULTRA_DEBUG

namespace LuaDebugger {

class LuaNetworkServerImpl : public LuaNetworkServer
{
public:
	LuaNetworkServerImpl();
	virtual ~LuaNetworkServerImpl();

	virtual bool Open( const char* serverInfoFileName, unsigned long serverPort );
	virtual void Close();

	virtual LuaNetworkSocket* GetSocket( const char* id );
	virtual void SendCommand( const char* id, const char* command );
	virtual void ProcessPackets();

	virtual void RegisterID( const char* id, void* userData, fnProcessCommand processCommand );
	virtual void* GetIDUserData( const char* id );

protected:
	struct IdInfo;

	struct SocketInfo
	{
		SimpleString m_id;
		SockAddr m_sockAddr;
		LuaNetworkSocket* m_socket;
	};

	void ReadAllConnectionsFromFileAndConnect();
	void WriteAllConnectionsToFile();
	void* GetAtSocketMap(const char* id);
	void SetAtSocketMap(SocketInfo& info);
	void* GetAtIdToStateMap(const char* id);
	void SetAtIdToStateMap(IdInfo& info);

	SimpleString m_serverInfoFileName;

	typedef SimpleList<SocketInfo> SocketMap;
	SocketMap m_socketMap;

	struct IdInfo
	{
		SimpleString m_id;
		void* userData;
		fnProcessCommand processCommand;
	};

	typedef SimpleList<IdInfo> IdToStateMap;
	IdToStateMap m_idToStateMap;

	unsigned long m_serverPort;

	enum State
	{
		WAITING_FOR_ACCEPT,
		VERIFY_ACCEPT,
		WAITING_FOR_CONNECT,
	};

	State m_state;
	LuaNetworkSocket m_acceptSocket;
	LuaNetworkSocket m_inSocket;
	SockAddr m_acceptSockAddr;
	char m_acceptID[32];
	LuaNetworkSocket* m_connectSocket;
	clock_t m_acceptStartTime;
};


const unsigned long MAGIC_NUMBER = 0xFEDC;

struct SocketConnectPacket
{
	unsigned long magicNumber;
	unsigned long port;
	char id[32];
};


LuaNetworkServer* LuaNetworkServer::CreateInstance()
{
	return new LuaNetworkServerImpl;
}


void LuaNetworkServer::DestroyInstance(LuaNetworkServer* server)
{
	delete server;
}

	
LuaNetworkServerImpl::LuaNetworkServerImpl() :
	m_serverPort(5001)
{
}


LuaNetworkServerImpl::~LuaNetworkServerImpl()
{
}


bool LuaNetworkServerImpl::Open(const char* serverInfoFileName, unsigned long serverPort)
{
	m_serverInfoFileName = serverInfoFileName;
	m_serverPort = serverPort;

	m_state = WAITING_FOR_ACCEPT;
	m_acceptSocket.Create(m_serverPort);
	ReadAllConnectionsFromFileAndConnect();
	
	return true;
}


void LuaNetworkServerImpl::Close()
{
}



LuaNetworkSocket* LuaNetworkServerImpl::GetSocket(const char* id)
{
	void* pos = m_socketMap.GetHeadPosition();
	while (pos)
	{
		SocketInfo& info = m_socketMap.GetNext(pos);
		if (info.m_id == id)
		{
			return info.m_socket;
		}
	}

	return NULL;
}

									
void LuaNetworkServerImpl::ProcessPackets()
{
	bool changed = false;

	// Have we received an accept request?
	if (m_state == WAITING_FOR_ACCEPT)
	{
		if (m_acceptSocket.IsReadDataWaiting())
		{
			m_inSocket = m_acceptSocket.Accept(&m_acceptSockAddr);
			if (m_inSocket.IsValid())
			{
				m_acceptStartTime = clock();
				m_state = VERIFY_ACCEPT;

#ifdef ULTRA_DEBUG			
				SockAddr::IPString buf;
				m_acceptSockAddr.GetIPAndPortString(buf);
				Log("LuaNetworkServerImpl: Incoming connection request from [%s]\n", buf);
#endif ULTRA_DEBUG
			}
		}
	}

	if (m_state == VERIFY_ACCEPT)
	{
		if (clock() - m_acceptStartTime < 3000)
		{
			unsigned long count = m_inSocket.GetReadDataWaiting();
			if (count >= sizeof(SocketConnectPacket))
			{
				SocketConnectPacket connectPacket;
				m_inSocket.Read(&connectPacket, sizeof(connectPacket));

				connectPacket.magicNumber = ntohl(connectPacket.magicNumber);

				if (connectPacket.magicNumber == MAGIC_NUMBER)
				{
					strncpy(m_acceptID, connectPacket.id, sizeof(m_acceptID));

					for (void* pos = m_socketMap.GetHeadPosition(); pos; m_socketMap.GetNext(pos))
					{
						SocketInfo& info = m_socketMap.GetAt(pos);
						if (info.m_id == connectPacket.id)
						{
							m_socketMap.RemoveAt(pos);
							break;
						}
					}

					// Connect
					m_acceptSockAddr.SetPort( ntohl(connectPacket.port) );

#ifdef ULTRA_DEBUG
					{
						SockAddr::IPString buf;
						m_acceptSockAddr.GetIPAndPortString(buf);
						Log("LuaNetworkServerImpl: Sending reconnect request [%s]\n", buf);
					}
#endif ULTRA_DEBUG
					m_connectSocket = new LuaNetworkSocket();

					// Start a non-blocking connect.
					if ( !m_connectSocket->Connect( m_acceptSockAddr, false ) )
					{
						delete m_connectSocket;
						m_state = WAITING_FOR_ACCEPT;
					}
					else
					{
						m_state = WAITING_FOR_CONNECT;
					}
				}
				else
				{
					// Doesn't match.  Get out of here.
					m_state = WAITING_FOR_ACCEPT;
				}
			}
		}
		else
		{
			m_state = WAITING_FOR_ACCEPT;
		}
	}

	if (m_state == WAITING_FOR_CONNECT)
	{
		if (m_connectSocket->IsWritable())
		{
#ifdef ULTRA_DEBUG			
			SockAddr::IPString buf;
			m_acceptSockAddr.GetIPAndPortString(buf);
			Log("LuaNetworkServerImpl: Connection made to [%s]\n", buf);
#endif ULTRA_DEBUG

			// Set the socket into non-blocking mode.
			m_connectSocket->SetToBlocking();

			m_connectSocket->WriteString(m_acceptID);

			LuaNetworkServerImpl::SocketInfo info;
			info.m_id = m_acceptID;
			info.m_sockAddr = m_acceptSockAddr;
			info.m_socket = m_connectSocket;

			SetAtSocketMap(info);

			SendCommand(m_acceptID, "LuaServerConnect()");
			WriteAllConnectionsToFile();

			m_state = WAITING_FOR_ACCEPT;
		}
	}
	
	for (void* pos = m_socketMap.GetHeadPosition(); pos; m_socketMap.GetNext(pos))
	{
		SocketInfo& info = m_socketMap.GetAt(pos);
		SimpleString id = info.m_id;

		void* idPos = GetAtIdToStateMap(id);
		IdInfo& idInfo = m_idToStateMap.GetAt(idPos);
		void* userData = idInfo.userData;
		if (!userData)
		{
			info.m_socket->Close();
		}
		else
		{
			while (info.m_socket->IsReadDataWaiting())
			{
				unsigned char* command;
				if (ReadBuffer(*info.m_socket, command))
				{
					// See if it is a wide string.
					unsigned short testShort = htons(0xfffe);
                    if (*(unsigned short*)command == testShort)
                    {
                        unsigned short* ptr = (unsigned short*)command;
                        while (*ptr)
                        {
                            *ptr = ntohs(*ptr);
                            ptr++;
                        }
                    }

					idInfo.processCommand(id, userData, command);
				}
				delete[] command;
			}
		}
	}

	if (changed)
	{
		WriteAllConnectionsToFile();
	}
}


void LuaNetworkServerImpl::SendCommand(const char* id, const char* command)
{
	LuaNetworkSocket* socket = GetSocket(id);
	if (!socket)
		return;
	socket->WriteString(command);
}


void LuaNetworkServerImpl::RegisterID(const char* id, void* userData, fnProcessCommand processCommand)
{
	IdInfo idInfo;
	idInfo.m_id = id;
	idInfo.userData = userData;
	idInfo.processCommand = processCommand;
	SetAtIdToStateMap(idInfo);
}


void* LuaNetworkServerImpl::GetIDUserData(const char* id)
{
	void* pos = GetAtIdToStateMap(id);
	return pos ? m_idToStateMap.GetAt(pos).userData : NULL;
}


void* LuaNetworkServerImpl::GetAtSocketMap(const char* id)
{
	for (void* pos = m_socketMap.GetHeadPosition(); pos; m_socketMap.GetNext(pos))
	{
		SocketInfo& info = m_socketMap.GetAt(pos);
		if (info.m_id == id)
		{
			return pos;
		}
	}

	return NULL;
}

									
void LuaNetworkServerImpl::SetAtSocketMap(SocketInfo& info)
{
	void* pos = GetAtSocketMap(info.m_id);
	if (pos)
	{
		SocketInfo& setAtInfo = m_socketMap.GetNext(pos);
		setAtInfo = info;
	}
	else
	{
		m_socketMap.AddTail(info);
	}
}

	
void* LuaNetworkServerImpl::GetAtIdToStateMap(const char* id)
{
	for (void* pos = m_idToStateMap.GetHeadPosition(); pos; m_idToStateMap.GetNext(pos))
	{
		IdInfo& info = m_idToStateMap.GetAt(pos);
		if (info.m_id == id)
		{
			return pos;
		}
	}

	return NULL;
}

									
void LuaNetworkServerImpl::SetAtIdToStateMap(IdInfo& info)
{
	void* pos = GetAtIdToStateMap(info.m_id);
	if (pos)
	{
		IdInfo& setAtInfo = m_idToStateMap.GetNext(pos);
		setAtInfo = info;
	}
	else
	{
		m_idToStateMap.AddTail(info);
	}
}

	
static char* TrimBuffer(char* buffer)
{
	char* start = buffer;
	while (*start != 0  &&  (*start == ' '  ||  *start == '\r'  ||  *start == '\n'  ||  *start == '\t'))
		start++;

	char* end = start + strlen(start) - 1;
	while (end > start  &&  (*end == ' '  ||  *end == '\r'  ||  *end == '\n'  ||  *end == '\t'))
		end--;

	end[1] = 0;

	return start;
}


void LuaNetworkServerImpl::ReadAllConnectionsFromFileAndConnect()
{
	FILE* file = fopen( m_serverInfoFileName, "rt" );
	if (file)
	{
		while ( !feof( file ) )
		{
			// Read the first line.
			char buffer[200];
			buffer[0] = 0;
			fgets(buffer, 200, file);

			char* start = TrimBuffer(buffer);

			if (*start == 0)
				break;

			SimpleString id = start;

			// Read the second line.
			fgets(buffer, 200, file);
			start = TrimBuffer(buffer);

			if (*start == 0)
				break;

			SimpleString ipStr = start;

			// Connect
			LuaNetworkSocket* socket = new LuaNetworkSocket;
			SockAddr sockAddr(ipStr);
			socket->Connect(sockAddr);
			socket->WriteString(id);
			socket->WriteString("LuaServerConnect()");

			LuaNetworkServerImpl::SocketInfo info;
			info.m_id = id;
			info.m_sockAddr = sockAddr;
			info.m_socket = socket;
			SetAtSocketMap(info);
		}

		fclose(file);
	}

	WriteAllConnectionsToFile();
}


void LuaNetworkServerImpl::WriteAllConnectionsToFile()
{
	FILE* file = fopen(m_serverInfoFileName, "wt");

	if (file)
	{
		for (void* pos = m_socketMap.GetHeadPosition(); pos; m_socketMap.GetNext(pos))
		{
			SocketInfo& info = m_socketMap.GetAt(pos);

			SockAddr::IPString ipStr;
			info.m_sockAddr.GetIPAndPortString(ipStr);
			fprintf(file, "%s\n%s\n", info.m_id.GetBuffer(), ipStr);
		}

		fclose(file);
	}
}

} // namespace LuaDebugger
