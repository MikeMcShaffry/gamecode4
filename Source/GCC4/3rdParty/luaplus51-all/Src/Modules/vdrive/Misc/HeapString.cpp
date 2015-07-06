#include "Misc_InternalPch.h"
#include "HeapString.h"
#include <stdarg.h>
#include <ctype.h>
#include "trio/trio.h"
extern "C"
{
#include "trio/triostr.h"
}

namespace Misc {

char nilHeapStringCharacter = '\0';


static const struct
{
    HeapStringData data;
    char dummy;
} g_heapStringEmpty = { {0, 0}, 0 };

const char* anEmptyHeapString = &g_heapStringEmpty.dummy;


void HeapStringData::Free()
{
	if ( this != (HeapStringData*)&g_heapStringEmpty )
		delete[] (unsigned char*)this;
}


static int CountAnsiHeapString(trio_pointer_t p, int)
{
	int& count = *(int*)p;
	count++;
	return 0;
}


HeapString HeapString::Format(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	int len = 0;
	trio_vcprintf(CountAnsiHeapString, &len, format, args);

	HeapString dest;
	dest.AllocBuffer(len);
	trio_vsnprintf(dest.m_string, len + 1, format, args);

	va_end(args);
	return dest;
}


HeapString HeapString::FormatV(const char* format, va_list args)
{
	int len = 0;
	trio_vcprintf(CountAnsiHeapString, &len, format, args);

	HeapString dest;
	dest.AllocBuffer(len);
	trio_vsnprintf(dest.m_string, len + 1, format, args);
	return dest;
}


const HeapString& HeapString::operator=(const HeapString& stringSrc)
{
    if (m_string == stringSrc.m_string)
		return *this;

	HeapStringData* srcStringData = stringSrc.GetStringData();

	if (srcStringData->stringLen == 0) {
		Clear();
		return *this;
	}

	HeapStringData* stringData = GetStringData();
	if (srcStringData->stringLen > stringData->maxSize)
		AllocBuffer(srcStringData->stringLen);

	memcpy(m_string, stringSrc.m_string, (srcStringData->stringLen + 1) * sizeof(char));

	if (srcStringData->stringLen > stringData->maxSize)
		stringData->Free();

	GetStringData()->stringLen = srcStringData->stringLen;

	return *this;
}


const HeapString& HeapString::operator=(const char* srcString)
{
	size_t srcStringLen = StrLen(srcString);
	if (srcStringLen == 0)
	{
		Clear();
		return *this;
	}

	HeapStringData* stringData = GetStringData();
	if (srcStringLen > stringData->maxSize)
		AllocBuffer(srcStringLen);

	memcpy(m_string, srcString, srcStringLen * sizeof(char));

	if (srcStringLen > stringData->maxSize)
		stringData->Free();

	GetStringData()->stringLen = srcStringLen;

	return *this;
}


const HeapString& HeapString::operator=(char ch)
{
	HeapStringData* stringData = GetStringData();
	if (1 > stringData->maxSize)
		AllocBuffer(1);

	m_string[0] = ch;

	if (1 > stringData->maxSize)
		stringData->Free();

	GetStringData()->stringLen = 1;

	return *this;
}


void HeapString::Clear()
{
	HeapStringData* pData = GetStringData();
	pData->Free();
	m_string = (char*)anEmptyHeapString;
}


HeapString HeapString::Sub(size_t startPos) const
{
	size_t stringLen = GetStringData()->stringLen;
	if (startPos >= stringLen)
		return HeapString();
	return Sub(startPos, stringLen - startPos);
}


HeapString HeapString::Sub(size_t startPos, size_t count) const
{
	size_t stringLen = GetStringData()->stringLen;
	if (startPos > stringLen  ||  count == 0)
		return HeapString();
	if (startPos + count > stringLen)
		count = stringLen - startPos;

	HeapString dest;
	dest.AllocBuffer(count);
	memcpy(dest.m_string, m_string + startPos, count * sizeof(char));
	return dest;
}


HeapString HeapString::Upper() const
{
	HeapString dest = *this;
	char* ptr = dest.m_string;
	while (*ptr != 0)
	{
		*ptr = toupper(*ptr);
		ptr++;
	}
	return dest;
}


HeapString HeapString::Lower() const
{
	HeapString dest = *this;
	char* ptr = dest.m_string;
	while (*ptr != 0)
	{
		*ptr = tolower(*ptr);
		ptr++;
	}
	return dest;
}


HeapString HeapString::TrimLeft(const char* trimChars) const
{
	HeapString dest;

	// Count trim characters.
	size_t trimCharsLen = 0;
	const char empty[] = { 0 };
	for (const char* ptr = trimChars ? trimChars : empty; *ptr; ++ptr, ++trimCharsLen) { }

	const char* str = m_string;
	if (trimCharsLen == 0)
	{
		while (isspace(*str))
			str++;
	}
	else
	{
		if (trimCharsLen == 1)
		{
			while (*str == *trimChars)
				str++;
		}
		else
		{
			const char* stringEnd = m_string + GetStringData()->stringLen;
			const char* lastStr = str;
			while (str != stringEnd)
			{
				for (const char* ptr = trimChars; *ptr; ++ptr)
				{
					if (*ptr == *str)
					{
						str++;
						break;
					}
				}
				if (lastStr == str)
					break;
				lastStr = str;
			}
		}
	}

	size_t count = GetStringData()->stringLen - (str - m_string);
	if (count > 0)
	{
		dest.AllocBuffer(count);
		memcpy(dest.m_string, str, count);
	}

	return dest;
}


HeapString HeapString::TrimRight(const char* trimChars) const
{
	HeapString dest;

	// Count trim characters.
	size_t trimCharsLen = 0;
	const char empty[] = { 0 };
	for (const char* ptr = trimChars ? trimChars : empty; *ptr; ++ptr, ++trimCharsLen) { }

	size_t stringLen = GetStringData()->stringLen;
	char* str = (char*)m_string + stringLen - 1;
	if (trimCharsLen == 0)
	{
		while (str != m_string  &&  isspace(*str))
			str--;
	}
	else
	{
		if (trimCharsLen == 1)
		{
			while (str != m_string  &&  *str == *trimChars)
				str--;
		}
		else
		{
			const char* lastStr = str;
			while (str != m_string)
			{
				for (const char* ptr = trimChars; *ptr; ++ptr)
				{
					if (*ptr == *str)
					{
						str--;
						break;
					}
				}
				if (lastStr == str)
					break;
				lastStr = str;
			}
		}
	}

	size_t count = str - m_string + 1;
	dest.AllocBuffer(count);
	memcpy(dest.m_string, m_string, count);
	return dest;
}


HeapString HeapString::Trim(const char* trimChars) const
{
	return TrimLeft(trimChars).TrimRight(trimChars);
}


int HeapString::Find(char ch, size_t start) const
{
	if (start >= GetStringData()->stringLen)
		return -1;

	const char* found = strchr(m_string + start, ch);
	return found ? (int)(found - m_string) : -1;
}


int HeapString::Find(const char* strSearch, size_t start) const
{
	if (start >= GetStringData()->stringLen)
		return -1;

	const char* found = strstr(m_string + start, strSearch);
	return found ? (int)(found - m_string) : -1;
}


int HeapString::ReverseFind(char ch) const
{
	const char* found = strrchr(m_string, ch);
	return found ? (int)(found - m_string) : -1;
}


int HeapString::FindOneOf(const char* charSet) const
{
	const char* found = strpbrk(m_string, charSet);
	return found ? (int)(found - m_string) : -1;
}


HeapString HeapString::BeforeLast(char ch) const
{
	HeapString str;
	int pos = ReverseFind(ch);
	if (pos == -1)
		str = *this;
	else
		str = HeapString(m_string, pos);

	return str;
}


HeapString HeapString::AfterLast(char ch) const
{
	HeapString str;
	int pos = ReverseFind(ch);
	if (pos == -1)
		str = *this;
	else
		str = m_string + pos + 1;

	return str;
}


HeapString HeapString::Replace(char oldCh, char newCh) const
{
	HeapString dest;
	dest.AllocBuffer(GetStringData()->stringLen);

	char* destPtr = dest.m_string;
	for (char* srcPtr = m_string; *srcPtr; ++srcPtr, ++destPtr)
	{
		char ch = *srcPtr;
		if (ch == oldCh)
			*destPtr = newCh;
		else
			*destPtr = ch;
	}

	return dest;
}


char* HeapString::GetBuffer(size_t nLen)
{
	HeapStringData* srcData = GetStringData();
	if (nLen > srcData->maxSize)
	{
		AllocBuffer(nLen);
		memcpy(m_string, srcData->data(), srcData->stringLen + 1);
		srcData->Free();
	}

	return m_string;
}

void HeapString::ReleaseBuffer()
{
	GetStringData()->stringLen = StrLen(m_string);
}

void HeapString::ReleaseBuffer(size_t nLen)
{
	m_string[nLen] = 0;
	GetStringData()->stringLen = nLen;
}


void HeapString::AllocBuffer(size_t nLen)
{
	if (nLen == 0)
	{
		m_string = (char*)anEmptyHeapString;
	}
	else
	{
		HeapStringData* pData = (HeapStringData*)
				new unsigned char[sizeof(HeapStringData) + (nLen+1)*sizeof(char)];
		pData->maxSize = nLen;
		pData->stringLen = nLen;
		m_string = pData->data();
		m_string[nLen] = '\0';
	}
}


HeapString& HeapString::ConcatCopy(size_t leftLen, const char* leftStr, size_t rightLen, const char* rightStr)
{
	size_t totalLen = leftLen + rightLen;
	if (totalLen == 0)
		return *this;

	AllocBuffer(totalLen);
	memcpy(m_string, leftStr, leftLen * sizeof(char));
	memcpy(m_string + leftLen, rightStr, rightLen * sizeof(char));
	return *this;
}


HeapString& HeapString::ConcatInPlace(int rightStringLen, const char* rightString)
{
	if (rightStringLen == 0)
		return *this;

	HeapStringData* origStringData = GetStringData();
	size_t origStringLen = origStringData->stringLen;
	if (origStringLen + rightStringLen > origStringData->maxSize)
	{
		const char* origString = m_string;
		AllocBuffer(origStringData->stringLen + rightStringLen);
		memcpy(m_string, origString, origStringLen * sizeof(char));
		memcpy(m_string + origStringLen, rightString, rightStringLen * sizeof(char));
		origStringData->Free();
		return *this;
	}

	memcpy(m_string + origStringLen, rightString, rightStringLen * sizeof(char));
	origStringLen += rightStringLen;
	origStringData->stringLen = origStringLen;
	m_string[origStringLen] = 0;
	return *this;
}


int HeapString::ToInt() const
{
	char* endp;
	return (int)trio_to_long(m_string, &endp, 10);
}


long HeapString::ToLong() const
{
	char* endp;
	return trio_to_long(m_string, &endp, 10);
}

} // namespace Misc

