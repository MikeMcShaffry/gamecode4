
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

#ifndef TILDE_INCLUDED_SendMessageBuffer_h
#define TILDE_INCLUDED_SendMessageBuffer_h

#include "tilde/HostConfig.h"

namespace tilde
{
	class SendMessageBuffer
	{
	public:

		SendMessageBuffer(int bufsize, int bufalign = 4)
			:
				m_bufsize(bufsize)
		{
			m_buffer = mem_alloc(bufsize, bufalign);
			m_position = 0;
			m_messageStart = -1;
			m_stringStart = -1;
		}

		~SendMessageBuffer()
		{
			mem_free(m_buffer);
		}

		void Begin()
		{
			TILDE_ASSERT(m_messageStart < 0);
			TILDE_ASSERT(m_stringStart < 0);
			m_messageStart = m_position;
			m_position += 4;
		}

		int End(const char * debugMessage)
		{
			int start = m_messageStart;
			int result = End();
			TILDE_PRINT("%s; sending msg length %d\n", debugMessage, m_position - start);
			return result;
		}

		int End()
		{
			TILDE_ASSERT(m_messageStart >= 0);
			TILDE_ASSERT(m_stringStart < 0);
			m_position = (int) round_up(m_position, 4);
			Write<int>((int) (m_position - m_messageStart), m_messageStart);
			m_messageStart = -1;
			return m_position;
		}

		bool IsSpaceAvailable(int required)
		{
			return required >= m_bufsize - m_position;
		}

		void * RawRead(int & size)
		{
			TILDE_ASSERT(m_messageStart < 0);
			TILDE_ASSERT(m_stringStart < 0);
			size = m_position;
			m_position = 0;
			m_messageStart = -1;
			m_stringStart = -1;
			return m_buffer;
		}

		bool IsEmpty() const
		{
			return m_position == 0;
		}

		void WriteValue(LuaDebuggerValue value)
		{
			Write<int>(value.GetType());
			if(value.GetType() == LuaType_NUMBER)
				Write<lua_Number>(value.GetNumber());
			else
				Write<LuaDebuggerObjectID>(value.GetObjectID());
		}

		void WriteString(const char * str)
		{
			int len = (str == NULL) ? 0 : (int) strlen(str) + 1;

			TILDE_ASSERT(m_messageStart >= 0);
			TILDE_ASSERT(m_stringStart < 0);
			TILDE_ASSERT(m_bufsize - m_position >= len + (int) sizeof(int));

			Write<int>((int) len);
			if(str != NULL)
				memcpy((char *) m_buffer + m_position, str, len);

			m_position += (int) round_up(len, 4);
		}

		void BeginString()
		{
			TILDE_ASSERT(m_messageStart >= 0);
			TILDE_ASSERT(m_stringStart < 0);
			m_stringStart = m_position;
			m_position += 4;
		}

		void AppendString(const char *fmt, ...)
		{
			TILDE_ASSERT(m_messageStart >= 0);
			TILDE_ASSERT(m_stringStart >= 0);		

			va_list	ap;
			va_start(ap,fmt);
			int result = vsnprintf((char *) m_buffer + m_position, m_bufsize - m_position, fmt, ap);
			va_end(ap);

			TILDE_ASSERT(result > 0);

			m_position += result;
		}

		void EndString()
		{
			TILDE_ASSERT(m_messageStart >= 0);
			TILDE_ASSERT(m_stringStart >= 0);
			Write<int>(m_position - m_stringStart - 4, m_stringStart);
			m_position = (int) round_up(m_position, 4);
			m_stringStart = -1;
		}

		template<typename T> 
		int Write(T data)
		{
			TILDE_ASSERT(m_messageStart >= 0);
			TILDE_ASSERT(m_stringStart < 0);
			TILDE_ASSERT(is_aligned(m_position, std::min<int>(4, sizeof(T))));
			TILDE_ASSERT(m_bufsize - m_position >= sizeof(T));
			int writePos = m_position;
			*reinterpret_cast<T *>( static_cast<char *>(m_buffer) + m_position) = to_network_order(data);
			m_position += sizeof(T);
			return writePos;
		}

		template<typename T> 
		int Write(T data, int pos)
		{
			TILDE_ASSERT(m_messageStart >= 0);
			TILDE_ASSERT(is_aligned(pos, std::min<int>(4, sizeof(T))));
			TILDE_ASSERT(pos >= 0);
			TILDE_ASSERT(m_bufsize - pos >= sizeof(T));
			*reinterpret_cast<T *>((char *) m_buffer + pos) = to_network_order(data);
			return pos;
		}

		int RawWrite(const void * data, int size)
		{
			TILDE_ASSERT(m_messageStart >= 0);
			TILDE_ASSERT(is_aligned(m_position, 4));
			TILDE_ASSERT(size >= 0);
			TILDE_ASSERT(m_bufsize - m_position >= size);
			int writePos = m_position;
			if(size >0 )
				memcpy(static_cast<char *>(m_buffer) + m_position, data, size);
			m_position += size;
			return writePos;
		}

		void	* m_buffer;
		int		m_bufsize;

		int		m_position;
		int		m_messageStart;
		int		m_stringStart;
	};

}	// namespace tilde

#endif // TILDE_INCLUDED_SendMessageBuffer_h
