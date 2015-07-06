#define BUILD_LUANETWORKSOCKET
#include <assert.h>
#include <stdio.h>

#ifdef WIN32
#include <winsock.h>
#undef SetPort
#pragma comment(lib, "ws2_32.lib")
#endif WIN32

#include "LuaNetworkHelpers.h"

#pragma warning(disable : 4127)

namespace LuaDebugger {

// From the Loki reference implementation
template<int> struct CompileTimeError;
template<> struct CompileTimeError<true> {};

////////////////////////////////////////////////////////////////////////////////
// macro STATIC_CHECK
// Invocation: STATIC_CHECK(expr, id)
// where:
// expr is a compile-time integral or pointer expression
// id is a C++ identifier that does not need to be defined
// If expr is zero, id will appear in a compile-time error message.
////////////////////////////////////////////////////////////////////////////////
#define STATIC_CHECK(expr, msg) \
    { CompileTimeError<((expr) != 0)> ERROR_##msg; (void)ERROR_##msg; }

void Log(const char* formatString, ...)
{
	const int BUF_SIZE = 800;
	char buf[BUF_SIZE];
	va_list arglist;

	va_start(arglist, formatString);
	int bufLen = _vsnprintf(buf, BUF_SIZE, formatString, arglist);
	if (bufLen == -1)
	{
		buf[BUF_SIZE - 2] = '\n';
		buf[BUF_SIZE - 1] = 0;
	}
	va_end(arglist);

	OutputDebugStringA(buf);
}


SimpleString::SimpleString() :
	m_buffer(NULL),
	m_len(0)
{
	Duplicate("");
}


SimpleString::SimpleString(const SimpleString& src) :
	m_buffer(NULL),
	m_len(0)
{
	Duplicate(src.m_buffer);
}


SimpleString::SimpleString(const char* str, size_t len) :
	m_buffer(NULL),
	m_len(0)
{
	Duplicate(str, len);
}


SimpleString::SimpleString(char ch) :
	m_buffer(NULL),
	m_len(0)
{
	char buf[2];
	buf[0] = ch;
	buf[1] = 0;
	Duplicate(buf);
}


SimpleString::~SimpleString()
{
	delete[] m_buffer;
}


SimpleString::operator const char*() const
{
	return m_buffer;
}


char* SimpleString::GetBuffer() const
{
	return m_buffer;
}


SimpleString& SimpleString::operator=(const SimpleString& src)
{
	Duplicate(src.m_buffer);
	return *this;
}


void SimpleString::Empty()
{
	Duplicate("");
}


char* SimpleString::Resize(size_t newLen)
{
	Duplicate(m_buffer, newLen);
	return m_buffer;
}


int SimpleString::Find(char ch)
{
	char* ptr = strchr(m_buffer, ch);
	if (ptr)
		return (int)(ptr - m_buffer);
	return -1;
}


int SimpleString::ReverseFind(char ch)
{
	char* ptr = strrchr(m_buffer, ch);
	if (ptr)
		return (int)(ptr - m_buffer);
	return -1;
}


int SimpleString::GetLength() const
{
	return (int)m_len;
}


SimpleString SimpleString::Left(size_t pos)
{
	return SimpleString(m_buffer, pos);
}


SimpleString SimpleString::Mid( int first ) const
{
	return (Mid(first, GetLength() - first));
}


SimpleString SimpleString::Mid(int first, int count) const
{
	if (first < 0)
		first = 0;
	if (count < 0)
		count = 0;

	if ((first + count) > GetLength())
	{
		count = GetLength() - first;
	}
	if (first > GetLength())
	{
		count = 0;
	}

	// optimize case of returning entire string
	if ((first == 0) && ((first + count) == GetLength()))
	{
		return *this;
	}

	return SimpleString(m_buffer + first, count);
}


bool SimpleString::IsEmpty() const
{
	return m_len == 0;
}


void SimpleString::MakeLower()
{
	strlwr(m_buffer);
}


void SimpleString::Replace(char oldCh, char newCh)
{
	for (size_t i = 0; i < m_len; ++i)
	{
		if (m_buffer[i] == oldCh)
			m_buffer[i] = newCh;
	}
}


SimpleString& SimpleString::operator+=(const SimpleString& src)
{
	Duplicate(m_buffer, m_len + src.m_len);
	strncpy(m_buffer + m_len, src.m_buffer, src.m_len);
	m_len += src.m_len;
	m_buffer[m_len] = 0;
	return *this;
}


SimpleString operator+( const SimpleString& str1, const SimpleString& str2 )
{
	SimpleString strResult = str1;
	strResult += str2;
	return strResult;
}


SimpleString operator+(const SimpleString& str1, const char* str2)
{
	SimpleString strResult = str1;
	strResult += str2;
	return strResult;
}


SimpleString operator+(const char* str1, const SimpleString& str2)
{
	SimpleString strResult = str1;
	strResult += str2;
	return strResult;
}


bool operator==( const SimpleString& str1, const SimpleString& str2 ) throw()
{
	return strcmp(str1.m_buffer, str2.m_buffer) == 0;
}


bool operator==(const SimpleString& str1, const char* str2) throw()
{
	return strcmp(str1.m_buffer, str2) == 0;
}


bool operator==(const char* str1, const SimpleString& str2 ) throw()
{
	return strcmp(str1, str2.m_buffer) == 0;
}


bool SimpleString::operator<(const SimpleString& src)
{
	return strcmp(m_buffer, src.m_buffer) < 0;
}


void SimpleString::Duplicate(const char* newStr, size_t requestedSize)
{
	size_t newStrLen = strlen(newStr);
	if (requestedSize != 0)
		newStrLen = newStrLen > requestedSize ? requestedSize : newStrLen;
	else
		requestedSize = newStrLen;
	char* newBuffer = (char*)malloc(requestedSize + 1);
	strncpy(newBuffer, newStr, newStrLen);
	newBuffer[newStrLen] = 0;

	if (m_buffer)
	{
		free(m_buffer);
	}

	m_buffer = newBuffer;
	m_len = newStrLen;
}


/**
	I think the original code of this was by Poul A. Costinsky.  I can't find
	a web address for his site anymore, though.
**/
SockAddr::SockAddr( unsigned char b1, unsigned char b2, unsigned char b3, unsigned char b4, unsigned short port )
{
	sockaddr_in& addr = GetSockAddr_In();
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_un_b.s_b1 = b1;
	addr.sin_addr.S_un.S_un_b.s_b2 = b2;
	addr.sin_addr.S_un.S_un_b.s_b3 = b3;
	addr.sin_addr.S_un.S_un_b.s_b4 = b4;
	addr.sin_port = htons( port );
}

SockAddr::SockAddr( unsigned long ip, unsigned short port )
{
	sockaddr_in& addr = GetSockAddr_In();
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_un_b.s_b1 = (u_char)(ip >> 24);
	addr.sin_addr.S_un.S_un_b.s_b2 = (u_char)((ip >> 16) & 0xFF);
	addr.sin_addr.S_un.S_un_b.s_b3 = (u_char)((ip >> 8) & 0xFF);
	addr.sin_addr.S_un.S_un_b.s_b4 = (u_char)((ip >> 0) & 0xFF);
	addr.sin_port = htons( port );
}

SockAddr::SockAddr( const char* ipStr )
{
	char s[80];
	strcpy( s, ipStr );
	char* ptr = strchr(s, ':');
	if (!ptr)
	{
		// Huh?
		assert(0);
	}
	*ptr = 0;
	unsigned int port = atol( ptr + 1 );

	*this = SockAddr( s, port );
}

SockAddr::SockAddr( const char* ipStr, unsigned int port )
{
	sockaddr_in& addr = GetSockAddr_In();
	memset( &addr, 0, sizeof(sockaddr_in) );
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_NONE;
	if ( ipStr )
		addr.sin_addr.s_addr = inet_addr( ipStr );
	if ( addr.sin_addr.s_addr == INADDR_NONE )
	{
		LPHOSTENT hostEnt;
		hostEnt = ::gethostbyname( ipStr );
		if ( hostEnt )
		{
			addr.sin_addr.s_addr = ((LPIN_ADDR)hostEnt->h_addr)->s_addr;
		}
	}

	addr.sin_port = htons( (u_short)port );
}

SockAddr::SockAddr( sockaddr_in* addr )
{
	GetSockAddr_In() = *addr;
}

void SockAddr::GetIPString( IPString& buf )
{
	sockaddr_in& addr = GetSockAddr_In();
	sprintf( buf, "%u.%u.%u.%u",
		addr.sin_addr.S_un.S_un_b.s_b1,
		addr.sin_addr.S_un.S_un_b.s_b2,
		addr.sin_addr.S_un.S_un_b.s_b3,
		addr.sin_addr.S_un.S_un_b.s_b4 );
}

void SockAddr::GetIPAndPortString( IPString& buf )
{
	sockaddr_in& addr = GetSockAddr_In();
	sprintf( buf, "%u.%u.%u.%u:%u",
		addr.sin_addr.S_un.S_un_b.s_b1,
		addr.sin_addr.S_un.S_un_b.s_b2,
		addr.sin_addr.S_un.S_un_b.s_b3,
		addr.sin_addr.S_un.S_un_b.s_b4,
		ntohs( addr.sin_port ) );
}

unsigned int SockAddr::GetPort()
{
	return ntohs( GetSockAddr_In().sin_port );
}

void SockAddr::SetPort(unsigned int port)
{
	GetSockAddr_In().sin_port = htons( (u_short)port );
}

sockaddr& SockAddr::GetSockAddr()
{
	STATIC_CHECK(sizeof(sockaddr) == SOCK_ADDR_SIZE, sockaddr_must_be_same_size);
	return *(sockaddr*)&sockAddr;
}


sockaddr_in& SockAddr::GetSockAddr_In()
{
	STATIC_CHECK(sizeof(sockaddr) == SOCK_ADDR_SIZE, sockaddr_must_be_same_size);
	return *(sockaddr_in*)&sockAddr;
}


LuaNetworkSocket::LuaNetworkSocket( SOCKET socket )
{
	WSADATA wsaData;
	WSAStartup( 0x0202, &wsaData );
	m_socket = socket;
}

LuaNetworkSocket::~LuaNetworkSocket()
{
	if ( m_socket != INVALID_SOCKET )
		Close();
	::WSACleanup();
}

LuaNetworkSocket& LuaNetworkSocket::operator=(const LuaNetworkSocket& src)
{
	Close();
	m_socket = src.m_socket;
	const_cast<LuaNetworkSocket&>(src).m_socket = INVALID_SOCKET;
	return *this;
}

LuaNetworkSocket::operator SOCKET() const
{
	return m_socket;
}

bool LuaNetworkSocket::Create( int port )
{
	m_socket = ::socket( PF_INET, SOCK_STREAM, 0 );
	if ( m_socket == INVALID_SOCKET )
		return false;

	SOCKADDR_IN	sockaddr;
	memset( &sockaddr, 0, sizeof( sockaddr ) );
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	sockaddr.sin_port = htons( (u_short)port );

	// Added begin...
	int val = 1;
	::setsockopt( m_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&val, sizeof( val ) );
	// Added end...

	int result = ::bind( m_socket, (SOCKADDR*)&sockaddr, sizeof( sockaddr ) );
	if (result == SOCKET_ERROR)
	{
		Close();
	}

	::setsockopt( m_socket, SOL_SOCKET, SO_DONTLINGER, (char *)&val, sizeof( val ) );
	::setsockopt( m_socket, SOL_SOCKET, SO_KEEPALIVE, (char *)&val, sizeof( val ) );

	result = ::listen( m_socket, 5 );
	return result == 0;
}


void LuaNetworkSocket::Close()
{
	if ( m_socket != INVALID_SOCKET )
	{
		::closesocket( m_socket );
		m_socket = INVALID_SOCKET;
	}
}

void LuaNetworkSocket::Attach( SOCKET socket )
{
	m_socket = socket;
}

/**
	SockAddr argument added from Poul's original implementation.
**/
LuaNetworkSocket LuaNetworkSocket::Accept( SockAddr* sockAddr )
{
	SockAddr sockAddrTemp;
	int sizeofSockAddr = sizeof( SockAddr );
	SOCKET ret = ::accept( m_socket, &sockAddrTemp.GetSockAddr(), &sizeofSockAddr );
	if ( ret == SOCKET_ERROR )
		return INVALID_SOCKET;

	if ( sockAddr )
		*sockAddr = sockAddrTemp;
	return ret;
}


/**
	Original implementation took a host address.  This one takes a SockAddr.
**/
bool LuaNetworkSocket::Connect( SockAddr& sockAddr, bool waitForConnect )
{
	if ( m_socket == INVALID_SOCKET )
	{
		m_socket = ::socket( PF_INET, SOCK_STREAM, 0 );
		if ( m_socket == INVALID_SOCKET )
			return false;
	}

	if (!waitForConnect)
	{
		// Set the socket into non-blocking mode.
		u_long val = 1;
		ioctlsocket(m_socket, FIONBIO, &val);

		int result = ::connect( m_socket, (SOCKADDR*)&sockAddr, sizeof( sockAddr ) );	result;
		return true;
	}

	int result = ::connect( m_socket, (SOCKADDR*)&sockAddr, sizeof( sockAddr ) );
	if ( result != SOCKET_ERROR )
	{
		assert( result == 0 );
		return true;
	}

	int e = ::WSAGetLastError();  e;
	assert( e != WSAEWOULDBLOCK );

	// Added.
	Close();

	return false;
}

bool LuaNetworkSocket::IsValid()
{
	return ( m_socket != INVALID_SOCKET );
}


void LuaNetworkSocket::SetToBlocking()
{
	if ( m_socket == INVALID_SOCKET )
		return;

	// Set the socket into non-blocking mode.
	u_long val = 0;
	ioctlsocket(m_socket, FIONBIO, &val);
}


// Added.
SockAddr LuaNetworkSocket::GetSockName()
{
	SockAddr sockAddr;
	int sockAddrLen = sizeof( SockAddr );
	int result = getsockname( m_socket, &sockAddr.GetSockAddr(), &sockAddrLen );
	if ( result == SOCKET_ERROR )
	{
		int e = ::WSAGetLastError();  e;
		assert( e != WSAEWOULDBLOCK );
	}
	return sockAddr;
}

	
// Added.
bool LuaNetworkSocket::IsReadDataWaiting()
{
	if ( m_socket == INVALID_SOCKET )
		return false;

	TIMEVAL timeval;
	timeval.tv_sec = 0;
	timeval.tv_usec = 0;

	fd_set readfds;
	fd_set exceptfds;
	FD_ZERO( &readfds );
	FD_SET( m_socket, &readfds );
	FD_ZERO( &exceptfds );
	FD_SET( m_socket, &exceptfds );

	int ret = ::select( 0, &readfds, NULL, NULL, &timeval );
	if ( ret == SOCKET_ERROR  ||  ret == 0 )
	{
		int e = WSAGetLastError(); e;
		assert( e != WSAEWOULDBLOCK );
		return false;
	}

	return FD_ISSET( m_socket, &readfds ) != 0;
}


// Added.
unsigned long LuaNetworkSocket::GetReadDataWaiting()
{
	if ( m_socket == INVALID_SOCKET )
		return 0;

	unsigned long count = 0;
	int ret = ioctlsocket( m_socket, FIONREAD, &count );  ret;
	return count;
}


bool LuaNetworkSocket::IsWritable()
{
	if ( m_socket == INVALID_SOCKET )
		return false;

	TIMEVAL timeval;
	timeval.tv_sec = 0;
	timeval.tv_usec = 0;

	fd_set writefds;
	FD_ZERO( &writefds );
	FD_SET( m_socket, &writefds );

	int ret = ::select( 0, NULL, &writefds, NULL, &timeval );
	if ( ret == SOCKET_ERROR  ||  ret == 0 )
	{
		int e = WSAGetLastError(); e;
//		assert( e != WSAEWOULDBLOCK );
		return false;
	}

	return FD_ISSET( m_socket, &writefds ) != 0;
}


/**
	read raw data
**/
int	LuaNetworkSocket::Read( void *pData, int len )
{
	if ( m_socket == INVALID_SOCKET )
		return 0;

	unsigned char* data = (unsigned char*)pData;
	int	countLeft = len;

	// if data size is bigger then network buffer
	// handle it nice

	do
	{
		int result = ::recv( m_socket, (char*)data, countLeft, 0 );
		if ( result == SOCKET_ERROR )
		{
			int e = WSAGetLastError();
			assert( e != WSAEWOULDBLOCK );
			if ( e == WSAECONNRESET  ||  e == WSAECONNABORTED )
			{
				Close();
			}
			return 0;
		}
		else if ( result == 0 )
		{
			// Socket has been closed.
			Close();
			return 0; 
		}
		else if ( result < 0 )
		{
			assert( 0 );
			return 0;
		}
		data += result;
		countLeft -= result;
	} while ( countLeft > 0 );

	assert( countLeft == 0 );
	return len;
}


/**
	write raw data
**/
int	LuaNetworkSocket::Write( const void *pData, size_t nLen )
{
	if ( m_socket == INVALID_SOCKET )
		return 0;

	unsigned char* data = (unsigned char*)pData;
	int countLeft = (int)nLen;

	// if data size is bigger then network buffer
	// handle it nice

	do
	{
		int result = ::send( m_socket, (char*)data, countLeft, 0 );
		if ( result == SOCKET_ERROR )
		{
			int e = WSAGetLastError();
			assert( e != WSAEWOULDBLOCK );
			if ( e == WSAECONNRESET )
			{
				Close();
			}
			return 0;
		}
		else if ( result == 0 )
			return 0;
		else if ( result < 0 )
		{
			assert( 0 );
			return 0;
		}
		data += result;
		countLeft -= result;
	} while ( countLeft > 0 );

	assert( countLeft == 0 );
	return countLeft;
}


/**
	Reads ANSI string from socket.
**/
int LuaNetworkSocket::ReadString( char* lpszString, int maxLen )
{
	u_long nt_Len;
	if ( Read( &nt_Len, sizeof( nt_Len ) ) < sizeof( nt_Len ) )
		return 0;
	int len = ntohl(nt_Len);
	if ( len == 0  ||  len >= maxLen )
		return 0;

	if ( Read( (void*)lpszString, len ) < len )
		return 0;
	lpszString[ len ] = 0;
	return len;
}


/**
	Writes ANSI string to socket.
**/
int LuaNetworkSocket::WriteString( const char* lpszString, int nLen )
{
	if ( m_socket == INVALID_SOCKET )
		return FALSE;

	if ( nLen < 0 )
		nLen = (int)strlen( lpszString );
	return WriteBinary( lpszString, nLen );
}


/**
	Writes a wide string to socket.
**/
int LuaNetworkSocket::WriteString( const wchar_t* lpszString, int nLen )
{
	if ( m_socket == INVALID_SOCKET )
		return FALSE;

	if ( nLen < 0 )
		nLen = (int)wcslen( lpszString );

	nLen++;
	nLen *= 2;

	u_long nt_Len = htonl(nLen);
	if ( Write( &nt_Len, sizeof( u_long ) ) != 0)
		return false;
	unsigned short c = htons(0xfffe);
	if ( Write( &c, sizeof(unsigned short) ) != 0 )
		return false;
	wchar_t* origBuf = new wchar_t[nLen];
	wchar_t* buf = origBuf;
	while (*lpszString)
	{
		*buf++ = htons(*lpszString++);
	}
	*buf = 0;
	if ( Write( origBuf, nLen - 2 ) != 0 )
	{
		delete[] origBuf;
		return false;
	}
	delete[] origBuf;
	return true;
}


// Added
bool LuaNetworkSocket::ReadBinary( unsigned char*& buffer, int& size )
{
	// Retrieve the binary size.
	if ( Read( &size, sizeof( unsigned long ) ) != sizeof( unsigned long ) )
	{
		// Ouch!
		buffer = NULL;
		size = 0;
		return false;
	}

	size = ntohl(size);
	
	buffer = new unsigned char[ size + 1 ];
	if ( Read( buffer, size ) != size )
	{
		// Ouch!
		delete [] buffer;
		buffer = NULL;
		size = 0;
		return false;
	}
	buffer[ size ] = 0;

	return true;
}


// Added.
bool LuaNetworkSocket::WriteBinary( const void* data, size_t size )
{
	size = htonl((u_long)size);
	if ( Write( &size, sizeof( size_t ) ) != 0)
		return false;
	size = ntohl((u_long)size);
	if ( Write( data, size ) != 0 )
		return false;
	return true;
}


int ReadString( LuaNetworkSocket& socket, SimpleString& str )
{
	str.Empty();

	if (!socket.IsValid())
		return 0;

	unsigned long nt_Len;
	if ( socket.Read( &nt_Len, sizeof(nt_Len) ) < sizeof( nt_Len ) )
		return 0;
	int len = ntohl(nt_Len);
	if ( len == 0 )
		return 0;

	char* buf = str.Resize( len );
	if ( socket.Read( (void*)buf, len ) < len )
		return 0;
	buf[len] = 0;
	return len;
}


int ReadBuffer( LuaNetworkSocket& socket, unsigned char*& buf )
{
	buf = NULL;

	if (!socket.IsValid())
		return 0;

	unsigned long nt_Len;
	if ( socket.Read( &nt_Len, sizeof(nt_Len) ) < sizeof( nt_Len ) )
		return 0;
	int len = ntohl(nt_Len);
	if ( len == 0 )
		return 0;

	buf = new unsigned char[len + 2];
	if ( socket.Read( (void*)buf, len ) < len )
		return 0;
	buf[len] = 0;
	buf[len + 1] = 0;
	return len;
}


void OsSleep(unsigned int millisecs)
{
	Sleep(millisecs);
}


} // namespace LuaDebugger
