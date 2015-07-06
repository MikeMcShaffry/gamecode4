
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

#ifndef TILDE_INCLUDED_ReceiveMessageBuffer_h
#define TILDE_INCLUDED_ReceiveMessageBuffer_h

#include <stdlib.h>
#include <string.h>

#include "lua.h"

#include "tilde/HostConfig.h"
#include "tilde/LuaDebugger.h"

namespace tilde
{
	class ReceiveMessageBuffer
	{
	public:
		ReceiveMessageBuffer(int size, int align = 4)
		{
			TILDE_ASSERT_MSG(is_aligned(size, align), "Circular buffer must have a size that is a multiple of the alignment");

			m_size = size;
			m_data = (u8 *) mem_alloc(size, align);
			m_readPosition = 0;
			m_readAvail = 0;
			m_writePosition = 0;
			m_writeAvail = size;
		}

		~ReceiveMessageBuffer()
		{
			mem_free(m_data);
		}

		int		GetSpaceAvailable() const		{ return m_writeAvail; }
		int		GetDataAvailable() const		{ return m_readAvail; }
		bool	IsEmpty() const					{ return m_readAvail == 0; }
		bool	IsFull() const					{ return m_writeAvail == 0; }
		bool	CanWrite(int count) const		{ return m_writeAvail >= count; }
		bool	CanRead(int count)				{ return m_readAvail >= count; }

		void Skip(int bytes)
		{
			TILDE_ASSERT(m_readAvail >= bytes);
			MoveReadPosition(bytes);
		}

		void AlignTo(int size)
		{
			int bytes = round_up(m_readPosition, size) - m_readPosition;
			Skip(bytes);
		}

		template<typename T> 
		T Peek() const
		{
			TILDE_ASSERT(is_aligned(m_readPosition, std::min<int>(4, sizeof(T))));
			TILDE_ASSERT(m_readAvail >= sizeof(T));
			return to_network_order(*reinterpret_cast<T *>(m_data + m_readPosition));
		}

		template<typename T> 
		T Read()
		{
			TILDE_ASSERT(is_aligned(m_readPosition, std::min<int>(4, sizeof(T))));
			TILDE_ASSERT(m_readAvail >= sizeof(T));
			T result = to_network_order(*reinterpret_cast<T *>(m_data + m_readPosition));
			MoveReadPosition(sizeof(T));
			return result;
		}

		template<typename T> 
		void Write(T data)
		{
			TILDE_ASSERT(is_aligned(m_writePosition, std::min<int>(4, sizeof(T))));
			TILDE_ASSERT(m_writeAvail >= sizeof(T));
			*reinterpret_cast<T *>(m_data + m_writePosition) = to_network_order(data);
		}

		LuaDebuggerValue ReadValue()
		{
			LuaType type = (LuaType) Read<int>();
			if(type == LuaType_NUMBER)
				return LuaDebuggerValue(type, Read<lua_Number>());
			else
				return LuaDebuggerValue(type, Read<LuaDebuggerObjectID>());
		}

		int ReadString(char * buffer, int bufsize)
		{
			int len = Read<int>();

			TILDE_ASSERT(m_readAvail >= len);
			TILDE_ASSERT(bufsize >= len + 1);
			int size1 = std::min(len, m_size - m_readPosition);
			memcpy(buffer, m_data + m_readPosition, size1);
			if(size1 < len)
				memcpy(buffer + size1, m_data, len - size1);
			buffer[len] = '\0';
			MoveReadPosition(round_up(len, 4));
			return len;
		}

		void PushString(lua_State * lvm)
		{
			int len = Read<int>();

			TILDE_ASSERT(m_readAvail >= len);
			int size1 = std::min(len, m_size - m_readPosition);
			lua_pushlstring(lvm, reinterpret_cast<const char *>(m_data + m_readPosition), size1);
			if(size1 < len)
			{
				lua_pushlstring(lvm, reinterpret_cast<const char *>(m_data), len - size1);
				lua_concat(lvm, 2);
			}
			MoveReadPosition(round_up(len, 4));
		}

		int Read(u8 * buffer, int count)
		{
			count = std::min(m_readAvail, count);
			int size1 = std::min(count, m_size - m_readPosition);
			memcpy(buffer, m_data + m_readPosition, size1);
			if(size1 < count)
				memcpy(buffer + size1, m_data, count - size1);
			MoveReadPosition(count);
			return count;
		}

		int Write(const u8 * data, int count)
		{
			TILDE_ASSERT(m_writeAvail >= count);
			int avail = std::min(count, m_writeAvail);
			int size1 = std::min(count, m_size - m_writePosition);

			memcpy(m_data + m_writePosition, data, size1);
			if(size1 < avail)
			{
				memcpy(m_data, data + size1, avail - size1);
			}
			MoveWritePosition(avail);
			return avail;
		}

		u8 * RawRead(int & size)
		{
			u8 * result = m_data + m_readPosition;
			size = std::min(m_readAvail, m_size - m_readPosition);
			MoveReadPosition(size);
			return result;
		}

	private:
	
		void	MoveReadPosition(int dist)
		{
			m_writeAvail += dist;
			m_readAvail -= dist;
			m_readPosition += dist;
			if(m_readPosition >= m_size)
				m_readPosition -= m_size;
		}

		void	MoveWritePosition(int dist)
		{
			m_writeAvail -= dist;
			m_readAvail += dist;
			m_writePosition += dist;
			if(m_writePosition >= m_size)
				m_writePosition -= m_size;
		}

	private:
		int	m_size;
		u8	* m_data;

		int	m_readPosition;
		int	m_writePosition;
		int	m_readAvail;
		int	m_writeAvail;
	};

}	// namespace tilde

#endif  // TILDE_INCLUDED_ReceiveMessageBuffer_h
