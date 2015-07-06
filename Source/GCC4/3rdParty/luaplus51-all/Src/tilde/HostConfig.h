
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

#ifndef TILDE_INCLUDED_HostConfig_h
#define TILDE_INCLUDED_HostConfig_h

#include <string>

#include "lua.h"


#define TILDE_ASSERT(exp)			((void)( (exp) != 0 || (tilde::assert_exp((const char *)#exp, __FILE__, __LINE__), 0) ))
#define TILDE_ASSERT_MSG(exp, ...)	((void)( (exp) != 0 || (tilde::assert_msg(__FILE__, __LINE__, __VA_ARGS__), 0) ))
//#define TILDE_PRINT(...)			tilde::print(__VA_ARGS__)
#define TILDE_PRINT(...)			((void)(0))

#define TILDE_SEND_BUFFER_SIZE		(512 * 1024)
#define TILDE_RECEIVE_BUFFER_SIZE	(4 * 1024)

#define TILDE_SEND_BUFFER_ALIGN		32
#define TILDE_RECEIVE_BUFFER_ALIGN	32

// Set this define if the host/target endianness is different
//#define TILDE_SWAP_ENDIAN

#ifdef min
#undef min
#endif

#ifdef WIN32
#define stricmp _stricmp
#endif

namespace tilde
{
	class ReceiveMessageBuffer;

	// ---------------------------------------------------------------------------------------------------------------
	// StlAllocator<T>
	// ---------------------------------------------------------------------------------------------------------------
/*
	template < typename _T > class StlAllocator : public std::allocator < _T >
	{
	public:
		StlAllocator()
		{
		}

		template < typename _U > struct rebind
		{
			typedef std::allocator < _U > other;
		};
	};
*/
#define StlAllocator std::allocator
	// ---------------------------------------------------------------------------------------------------------------
	// types
	// ---------------------------------------------------------------------------------------------------------------

	typedef char				s8;
	typedef short				s16;
	typedef int					s32;
	typedef long long			s64;
	typedef unsigned char		u8;
	typedef unsigned short		u16;
	typedef unsigned int		u32;
	typedef unsigned long long	u64;

	typedef std::basic_string < char, std::char_traits < char >, StlAllocator < char > > String;

	typedef size_t				LuaDebuggerObjectID;

	// ---------------------------------------------------------------------------------------------------------------
	// Helper functions
	// ---------------------------------------------------------------------------------------------------------------

	void * mem_alloc(size_t length, int align);
	void mem_free(void * ptr);
	void assert_exp(const char *exp, const char *file, unsigned line);
	void assert_msg(const char *file, unsigned line, const char *fmt, ...);
	void print(const char * fmt, ...);

	inline int round_up(int num, int align)
	{
		return (num + align - 1) & ~(align - 1);
	}

	inline int is_aligned(int num, int align)	
	{
		return (num & (align - 1)) == 0;
	}



#ifndef TILDE_SWAP_ENDIAN

	template <typename T>
	inline T to_network_order(T v)
	{
		return v;
	}

#else

	template <typename T> inline T to_network_order(const T value)
	{
		T result;
		const u8 * in = reinterpret_cast<const u8 *>(&value);
		u8 * out = reinterpret_cast<u8 *>(&result);

		for(int index = 0; index < sizeof(T); ++ index)
			out[index] = in[sizeof(T) - 1 - index];

		return result;
	}

#endif // TILDE_SWAP_ENDIAN

	void OsSleep(unsigned int millisecs);

}

#endif // TILDE_INCLUDED_HostConfig_h
