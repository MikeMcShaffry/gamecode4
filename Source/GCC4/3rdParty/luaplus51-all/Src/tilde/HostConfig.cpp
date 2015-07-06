
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

#include "HostConfig.h"

#include <stdio.h>
#include <string.h>
#if defined(WIN32)
	#include <windows.h>
#endif


namespace tilde
{
	void * mem_alloc(size_t length, int align)
	{
		return (void *) malloc(length);
	}

	void mem_free(void * ptr)
	{
		free(ptr);
	}

	char s_printBuffer[1024];

	void assert_exp(const char *exp, const char *file, unsigned line)
	{
#if defined(WIN32)
		_snprintf(s_printBuffer, sizeof(s_printBuffer), "Assertion failed at %s:%d; %s\n", file, line, exp);

		OutputDebugString(s_printBuffer);
		printf(s_printBuffer);
		DebugBreak();
		exit(1);
#else
		assert(0);
#endif // defined(WIN32)
	}

	void assert_msg(const char *file, unsigned line, const char *fmt, ...)
	{
#if defined(WIN32)
		va_list	ap;

		_snprintf(s_printBuffer, sizeof(s_printBuffer), "Assertion failed at %s:%d; ", file, line);
		int len = (int) strlen(s_printBuffer);

		va_start(ap, fmt);
		vsnprintf(s_printBuffer + len, sizeof(s_printBuffer) - len, fmt, ap);
		va_end(ap);

		strcat(s_printBuffer, "\n");

		OutputDebugString(s_printBuffer);
		printf(s_printBuffer);
		DebugBreak();
		exit(1);
#else
		assert(0);
#endif // defined(WIN32)
	}

	void print(const char * fmt, ...)
	{
#if defined(WIN32)
		va_list	ap;

		va_start(ap, fmt);
		vsnprintf(s_printBuffer, sizeof(s_printBuffer), fmt, ap);
		va_end(ap);

		OutputDebugString(s_printBuffer);
		printf(s_printBuffer);
#endif // defined(WIN32)
	}
}
