
/****************************************************************************

Tilde

Copyright (c) 2008 Tantalus Media Pty

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

****************************************************************************/

#include "LuaHostWindows.h"

#include <stdio.h>

#if defined(WIN32)
	#include <winsock2.h>

	#pragma comment(lib, "wsock32.lib")

	#define SocketError					WSAGetLastError()

	typedef int socklen_t;
#endif

#include "LuaHostWindows.h"

#include "tilde/LuaDebugger.h"
#include "tilde/LuaDebuggerComms.h"
#include "tilde/ReceiveMessageBuffer.h"

#include "lua.h"
#include "lobject.h"

#define strcmp _stricmp

const char * FormatAddress(sockaddr_in & address)
{
	static char buf[256];
	strcpy(buf, inet_ntoa(address.sin_addr));
	sprintf(buf + strlen(buf), ":%d", address.sin_port);
	return buf;
}

char s_printBuffer[1024];

void print(const char * format, ...)
{
	if (1) return;

	va_list	ap;

	va_start(ap,format);
	vsnprintf(s_printBuffer, 1024, format, ap);
	va_end(ap);

#if defined(WIN32)
	OutputDebugString(s_printBuffer);
#endif // WIN32
	printf(s_printBuffer);
}

void warn(const char * format, ...)
{
	if (1) return;

	va_list	ap;

	print("WARNING: ");

	va_start(ap,format);
	vsnprintf(s_printBuffer, 1024, format, ap);
	va_end(ap);

#if defined(WIN32)
	OutputDebugString(s_printBuffer);
#endif // defined(WIN32)
	printf(s_printBuffer);

	print("\n");
}

void error(const char * format, ...)
{
	if (1) return;

	va_list	ap;

	print("FATAL ERROR: ");

	va_start(ap,format);
	vsnprintf(s_printBuffer, 1024, format, ap);
	va_end(ap);

#if defined(WIN32)
	OutputDebugString(s_printBuffer);
#endif // defined(WIN32)
	printf(s_printBuffer);

	print("\n");

#if defined(WIN32)
	DebugBreak();
	exit(1);
#endif // defined(WIN32)
}


namespace tilde {

void OsSleep(unsigned int millisecs)
{
#if defined(WIN32)
	Sleep(millisecs);
#elif defined(__CELLOS_LV2__)
	sys_timer_usleep( (millisecs) * 1000 );
#endif // defined(WIN32)
}


class LuaHostWindows::Detail
{
public:
	Detail(LuaDebuggerHost* host, int port);
	~Detail();

	void	InitialiseServerSocket();
	void	DestroyServerSocket();
	void	Close();

	void	Poll();

	tilde::LuaDebuggerComms * m_debuggerComms;

	int		m_serverPort;

	SOCKET	m_serverSocket;
	SOCKET	m_debuggerSocket;

	unsigned char	* m_netBuffer;
	int				m_netBufferSize;

	tilde::String	m_functionName;
	tilde::String	m_targetFileName;
};


LuaHostWindows::Detail::Detail(LuaDebuggerHost* host, int port)
	:
		m_serverPort(port),
		m_serverSocket(SOCKET_ERROR),
		m_debuggerSocket(SOCKET_ERROR)
{
	m_debuggerComms = new tilde::LuaDebuggerComms(host);

	m_netBufferSize = 4096;
	m_netBuffer = new tilde::u8[m_netBufferSize];

	InitialiseServerSocket();
}


LuaHostWindows::Detail::~Detail()
{
	delete[] m_netBuffer;
	delete m_debuggerComms;
}

	
void LuaHostWindows::Detail::InitialiseServerSocket()
{
#if defined(WIN32)
	// Initialize Winsock.
	WSADATA wsaData;
	int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
	if(iResult != NO_ERROR)
		error("WSAStartup() failed (error %d)", WSAGetLastError());
#endif // defined(WIN32)

	// Create a socket.
	m_serverSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if(m_serverSocket == INVALID_SOCKET) 
	{
		error("socket() failed (error %d)", SocketError);
#if defined(WIN32)
		WSACleanup();
#endif // defined(WIN32)
		return;
	}

	// Bind the socket.
	sockaddr_in service;

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_port = htons( m_serverPort );

	if (bind(m_serverSocket, (struct sockaddr*) &service, sizeof(service) ) == SOCKET_ERROR ) 
	{
		error("bind() failed (error %d)", SocketError);
		closesocket(m_serverSocket);
		return;
	}

	// Listen on the socket.
	if (listen( m_serverSocket, 1 ) == SOCKET_ERROR)
		error("listen() failed (error %d)", SocketError);
	else
		fprintf(stderr, "Listening for debugger connection on port %d...\n", m_serverPort);
}

void LuaHostWindows::Detail::DestroyServerSocket()
{
	if(m_serverSocket != SOCKET_ERROR)
	{
		closesocket(m_serverSocket);
		m_serverSocket = SOCKET_ERROR;
	}

#if defined(WIN32)
	WSACleanup();
#endif
}


void LuaHostWindows::Detail::Close()
{
	m_debuggerComms->Close();
	m_debuggerSocket = SOCKET_ERROR;
}


void LuaHostWindows::Detail::Poll()
{
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(m_serverSocket, &readfds);

	if(m_debuggerSocket != SOCKET_ERROR)
		FD_SET(m_debuggerSocket, &readfds);

	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	int count = select(0, &readfds, NULL, NULL, &timeout);

	if(count == SOCKET_ERROR)
		error("select() failed (error %d)", SocketError);

	else if(count >= 1)
	{
		if(FD_ISSET(m_debuggerSocket, &readfds) && m_debuggerSocket != SOCKET_ERROR)
		{
			int bytes = recv(m_debuggerSocket, (char *) m_netBuffer, m_netBufferSize, 0);
			if(bytes == 0)
				Close();
			else if(bytes == SOCKET_ERROR)
			{
				warn("recv() failed (error %d)", SocketError);
				Close();
			}
			else
			{
				m_debuggerComms->Receive(m_netBuffer, bytes);
			}
		}

		if(FD_ISSET(m_serverSocket, &readfds) && m_debuggerSocket == SOCKET_ERROR && !m_debuggerComms->GetDebugger()->IsConnected())
		{
			sockaddr_in address;
			socklen_t addressLen = sizeof(address);
			m_debuggerSocket = accept(m_serverSocket, (sockaddr *) &address, &addressLen);

			if(m_debuggerSocket == SOCKET_ERROR)
				error ("accept() failed (error %d)", SocketError);

			print("Connection accepted from %s!\n", FormatAddress(address));

			m_debuggerComms->Open();
		}
	}
}


LuaHostWindows::LuaHostWindows(int port)
	:
		m_detail(NULL)
{
	m_detail = new Detail(this, port);
}

LuaHostWindows::~LuaHostWindows()
{
	delete m_detail;
}

bool LuaHostWindows::IsConnected() const
{
	return m_detail->m_debuggerComms->GetDebugger()->IsConnected();
}

void LuaHostWindows::RegisterState(const char* name, lua_State* lvm)
{
	m_detail->m_debuggerComms->RegisterState(name, lvm);
}

void LuaHostWindows::WaitForDebuggerConnection()
{
	while (!IsConnected())
	{
		Poll();
		OsSleep(50);
	}
}

void LuaHostWindows::Poll()
{
	m_detail->Poll();
}

void LuaHostWindows::SendDebuggerData( const void * data, int size )
{
	if(m_detail->m_debuggerSocket != SOCKET_ERROR)
	{
		send(m_detail->m_debuggerSocket, (char *) data, (int) size, 0);
	}
}

void LuaHostWindows::CloseDebuggerConnection()
{
	m_detail->Close();
}

bool LuaHostWindows::AttachLuaHook( lua_State* lvm, lua_Hook hook, int mask, int count )
{
	lua_sethook(lvm, hook, mask, count);
	return true;
}

void LuaHostWindows::DetachLuaHook( lua_State* lvm, lua_Hook hook )
{
	lua_sethook(lvm, NULL, 0, 0);
}

void LuaHostWindows::AttachPrintfHook( void (* hook)(const char *) )
{

}

void LuaHostWindows::DetachPrintfHook( void (* hook)(const char *) )
{

}

void LuaHostWindows::ReceiveExCommand(const char * command, int datalen, tilde::ReceiveMessageBuffer * data)
{
	data->Skip(datalen);
}

void LuaHostWindows::OnIdle()
{
	Poll();
}

void LuaHostWindows::OnRun()
{
	Poll();
}

const char * LuaHostWindows::GetFunctionName( lua_State * lvm, int funcIndex, lua_Debug * ar )
{
	tilde::String& functionName = m_detail->m_functionName;
	functionName.clear();

	// Start with the name the function was called by
	if (ar->name)
		functionName.append(ar->name);

	// If it's a C function, then add its details
	if(lua_iscfunction(lvm, funcIndex))
	{
		if(!functionName.empty())
			functionName.append(" ");

		CClosure * closure = (CClosure *) lua_topointer(lvm, funcIndex);
		char buf[16];
		sprintf(buf, "%p", closure->f);
		functionName.append("(C function ");
		functionName.append(buf);
		functionName.append(")");
	}

	// If it's a lua function then add the file/line
	if (ar->source && ar->linedefined >= 0)
	{
		char buf[16];

		if(ar->currentline >= 0)
			sprintf(buf, "%d", ar->currentline);
		else
			sprintf(buf, "%d", ar->linedefined);

		if(!functionName.empty())
			functionName.append(" ");

		functionName.append("(");
		functionName.append(ar->source);
		functionName.append(":");
		functionName.append(buf);
		functionName.append(")");
	}

	if (ar->what)
	{
		if(!functionName.empty())
			functionName.append(" ");

		functionName.append("[");
		functionName.append(ar->what);
		functionName.append("]");
	}

	return functionName.c_str();
}

const char * LuaHostWindows::LookupSourceFile(const char *target)
{
	if(target[0] == '@')
		return target + 1;
	else
		return target;
}

const char * LuaHostWindows::LookupTargetFile(const char *source)
{
	m_detail->m_targetFileName.clear();
	m_detail->m_targetFileName.append("@");
	m_detail->m_targetFileName.append(source);
	return m_detail->m_targetFileName.c_str();
}


} // namespace tilde
