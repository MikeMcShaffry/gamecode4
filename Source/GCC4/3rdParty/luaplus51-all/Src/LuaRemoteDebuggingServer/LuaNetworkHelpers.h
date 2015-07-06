#ifndef LUANETWORKHELPERS_H
#define LUANETWORKHELPERS_H

#pragma once

#pragma warning(disable : 4512)

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#undef SetPort

#if _MSC_VER  &&  _MSC_VER < 1300
typedef unsigned int           SOCKET;
#else
typedef UINT_PTR SOCKET;
#endif

struct sockaddr_in;
struct sockaddr;

namespace LuaDebugger {

class SimpleString
{
public:
	SimpleString();
	SimpleString(const SimpleString& src);
	SimpleString(const char* str, size_t len = 0);
	SimpleString(char ch);
	~SimpleString();

	operator const char*() const;
	char* GetBuffer() const;
	SimpleString& operator=(const SimpleString& src);

	void Empty();
	char* Resize(size_t newLen);
	int Find(char ch);
	int ReverseFind(char ch);
	int GetLength() const;
	SimpleString Left(size_t pos);
	SimpleString Mid( int first ) const;
	SimpleString Mid(int first, int count) const;
	bool IsEmpty() const;
	void MakeLower();
	void Replace(char oldCh, char newCh);

	SimpleString& operator+=(const SimpleString& src);
	friend SimpleString operator+( const SimpleString& str1, const SimpleString& str2 );
	friend SimpleString operator+(const SimpleString& str1, const char* str2);
	friend SimpleString operator+(const char* str1, const SimpleString& str2);

	friend bool operator==(const SimpleString& str1, const SimpleString& str2) throw();
	friend bool operator==(const SimpleString& str1, const char* psz2) throw();
	friend bool operator==(const char* psz1, const SimpleString& str2) throw();

	bool operator<(const SimpleString& src);

protected:
	void Duplicate(const char* newStr, size_t requestedSize = 0);

	char* m_buffer;
	size_t m_len;
};


template<typename E>
class SimpleList
{
public:
	SimpleList() :
		m_pHead(NULL),
		m_pTail(NULL),
		m_numElements(0)
	{
	}

	~SimpleList() throw()
	{
		RemoveAll();
	}

	void AddTail(const E& element)
	{
		CNode* pNewNode = new CNode(element);
		pNewNode->m_pPrev = m_pTail;
		pNewNode->m_pNext = NULL;

		if (m_pTail)
			m_pTail->m_pNext = pNewNode;
		else
			m_pHead = pNewNode;

		m_pTail = pNewNode;
		m_numElements++;
	}

	void* AddTail()
	{
		CNode* pNewNode = new CNode();
		pNewNode->m_pPrev = m_pTail;
		pNewNode->m_pNext = NULL;

		if (m_pTail)
			m_pTail->m_pNext = pNewNode;
		else
			m_pHead = pNewNode;

		m_pTail = pNewNode;
		m_numElements++;

		return (void*)pNewNode;
	}

	void RemoveAt(void* pos)
	{
		CNode* pOldNode = (CNode*)pos;

		// remove pOldNode from list
		if( pOldNode == m_pHead )
		{
			m_pHead = pOldNode->m_pNext;
		}
		else
		{
			pOldNode->m_pPrev->m_pNext = pOldNode->m_pNext;
		}
		if( pOldNode == m_pTail )
		{
			m_pTail = pOldNode->m_pPrev;
		}
		else
		{
			pOldNode->m_pNext->m_pPrev = pOldNode->m_pPrev;
		}
		FreeNode( pOldNode );
	}

	void RemoveAll()
	{
		while (m_pHead)
		{
			CNode* pKill = m_pHead;
			m_pHead = m_pHead->m_pNext;
			delete pKill;
		}

		m_pHead = NULL;
		m_pTail = NULL;
		m_numElements = 0;
	}

	size_t GetCount() const throw()
	{
		return m_numElements;
	}

	void* GetHeadPosition() const throw()
	{
		return m_pHead;
	}

	E& GetNext( void*& pos ) throw()
	{
		CNode* pNode = (CNode*)pos;
		pos = (void*)pNode->m_pNext;
		return pNode->m_element;
	}

	E& GetAt(void* pos) throw()
	{
		CNode* pNode;

		pNode = (CNode*)pos;

		return( pNode->m_element );
	}

	const E& GetAt(void* pos) const throw()
	{
		CNode* pNode;

		pNode = (CNode*)pos;

		return( pNode->m_element );
	}

	void Sort()
	{
		// Exit if not enough items.
		if (!m_pHead  ||  m_pHead->m_pNext == NULL)
			return;

		CNode* curNode = m_pHead->m_pNext;
		while (curNode)
		{
			CNode* curCheckNode = curNode;
			while (curCheckNode->m_pPrev  &&  curNode->m_element < curCheckNode->m_pPrev->m_element)
			{
				curCheckNode = curCheckNode->m_pPrev;
			}

			CNode* nextNode = curNode->m_pNext;

			if (curNode != curCheckNode)
			{
				// Remove curNode from the list.
				if (curNode == m_pTail)
				{
					m_pTail = curNode->m_pPrev;
				}
				else
				{
					curNode->m_pNext->m_pPrev = curNode->m_pPrev;
				}

				curNode->m_pPrev->m_pNext = curNode->m_pNext;

				// Insert curNode to the new location.
				if (curCheckNode->m_pPrev)
				{
					curCheckNode->m_pPrev->m_pNext = curNode;
				}
				else
				{
					m_pHead = curNode;
				}

				curNode->m_pPrev = curCheckNode->m_pPrev;
				curNode->m_pNext = curCheckNode;
				curCheckNode->m_pPrev = curNode;
			}

			curNode = nextNode;
		}
	}

private:
	class CNode
	{
	public:
		CNode() {}
		CNode( const E& element ) : m_element( element )  { }

		CNode* m_pNext;
		CNode* m_pPrev;
		E m_element;

	private:
		CNode( const CNode& ) throw();
	};

	void FreeNode( CNode* pNode ) throw()
	{
		delete pNode;
		m_numElements--;
	}

	CNode* m_pHead;
	CNode* m_pTail;
	size_t m_numElements;

	SimpleList( const SimpleList& ) throw();
	SimpleList& operator=( const SimpleList& ) throw();
};


enum { SOCK_ADDR_SIZE = 16 };

/**
	I think the original code of this was by Poul A. Costinsky.  I can't find
	a web address for his site anymore, though.
**/
class SockAddr
{
public:
	typedef char IPString[3 + 1 + 3 + 1 + 3 + 1 + 3 + 1 + 5 + 1]; // 123.123.123.123:12345

	SockAddr( unsigned char b1 = 0, unsigned char b2 = 0, unsigned char b3 = 0, unsigned char b4 = 0, unsigned short port = 0 );
	SockAddr( unsigned long ip, unsigned short port );
	SockAddr( const char* ipStr );
	SockAddr( const char* ipStr, unsigned int port );
	SockAddr( sockaddr_in* addr );
	void GetIPString( IPString& buf );
	void GetIPAndPortString( IPString& buf );
	unsigned int GetPort();
	void SetPort(unsigned int port);

	sockaddr& GetSockAddr();
	sockaddr_in& GetSockAddr_In();

protected:
#ifdef BUILD_LUANETWORKSOCKET
	struct sockaddr sockAddr;
#else !BUILD_LUANETWORKSOCKET
	unsigned char sockAddr[SOCK_ADDR_SIZE];
#endif BUILD_LUANETWORKSOCKET
};


/**
	Original code by Poul A. Costinsky.  I can't find a web address for it,
	but it had something to do with a RawSocketServerWorker class.

	Added code includes:

		- automatic WinSock initialization.
		-
**/
class LuaNetworkSocket
{
public:
#if _MSC_VER  &&  _MSC_VER < 1300
	static enum { INVALID = -1 };
#else
	static const SOCKET INVALID = (SOCKET)(~0);
#endif

	LuaNetworkSocket( SOCKET socket = INVALID );
	~LuaNetworkSocket();
	LuaNetworkSocket& operator=(const LuaNetworkSocket& src);

	operator SOCKET() const;

	bool Create( int port );
	void Close();
	void Attach( SOCKET socket );
	LuaNetworkSocket Accept( SockAddr* sockAddr = NULL );
	bool Connect( SockAddr& sockAddr, bool waitForConnect = true );
	bool IsValid();
	void SetToBlocking();
	SockAddr GetSockName();
	bool IsReadDataWaiting();
	unsigned long GetReadDataWaiting();
	bool IsWritable();
	int	Read( void *pData, int len );
	int	Write( const void *pData, size_t nLen );
	int ReadString( char* lpszString, int maxLen );
	int WriteString( const char* lpszString, int nLen = -1 );
	int WriteString( const wchar_t* lpszString, int nLen = -1 );
	bool ReadBinary( unsigned char*& buffer, int& size );
	bool WriteBinary( const void* data, size_t size );

protected:
	SOCKET m_socket;
};

extern void Log(const char* formatStr, ...);

int ReadString( LuaNetworkSocket& socket, SimpleString& str );
int ReadBuffer( LuaNetworkSocket& socket, unsigned char*& buf );
void OsSleep(unsigned int millisecs);

} // namespace LuaDebugger

#endif LUANETWORKHELPERS_H

