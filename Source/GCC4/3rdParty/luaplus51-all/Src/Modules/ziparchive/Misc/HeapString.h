#ifndef MISC_HEAPSTRING_H
#define MISC_HEAPSTRING_H

#include "Misc.h"
#include "TypeTraits.h"
#include <memory.h>
#include <string.h>
#include <stdarg.h>

namespace Misc {

struct HeapStringData
{
    size_t stringLen;
	size_t maxSize;

    char* data() const   { return (char*)(this+1); }

    void  Free();
};


extern const char* anEmptyHeapString;


class HeapString
{
public:
	static HeapString Format(const char* format, ...);
	static HeapString FormatV(const char* format, va_list args);

	HeapString()										{  m_string = (char*)anEmptyHeapString;  }
    HeapString(const HeapString& stringSrc)
    {
        if ( stringSrc.IsEmpty() )
        {
            m_string = (char*)anEmptyHeapString;
        }
        else
        {
            AllocBuffer(stringSrc.Length());
            memcpy(m_string, stringSrc.m_string, stringSrc.Length());
        }
    }

    HeapString(size_t n, char ch)
	{
		m_string = (char*)anEmptyHeapString;
		if (n >= 1)
		{
			AllocBuffer(n);
			for (size_t i = 0; i < n; ++i)
				m_string[i] = ch;
		}
	}

	HeapString(const char* str)
    {
        m_string = (char*)anEmptyHeapString;
        size_t len = StrLen(str);
        if (len > 0)
        {
            AllocBuffer(len);
            memcpy(m_string, str, len);
        }
    }

	HeapString(const char* str, size_t len)
    {
		m_string = (char*)anEmptyHeapString;
		if (len == (size_t)-1)
			len = StrLen(str);
		if (len > 0)
		{
			AllocBuffer(len);
			memcpy(m_string, str, len);
		}
    }

	HeapString(char ch)
	{
		m_string = (char*)anEmptyHeapString;
		AllocBuffer(1);
		m_string[0] = ch;
		m_string[1] = 0;
	}

	explicit HeapString(const wchar* str)
    {
        m_string = (char*)anEmptyHeapString;
        size_t len = 0;
		while (str[len] != 0) len++;
        if (len > 0)
        {
            AllocBuffer(len);
			for (size_t i = 0; i < len; ++i)
				m_string[i] = (char)(BYTE)str[i];
        }
    }

	~HeapString()
    {
		GetStringData()->Free();
    }

    const HeapString& operator=(const HeapString& stringSrc);
    const HeapString& operator=(const char* srcString);
    const HeapString& operator=(char ch);

    size_t Length() const							{  return GetStringData()->stringLen;  }
    bool IsEmpty() const							{  return GetStringData()->stringLen == 0;  }
    bool IsNotEmpty() const							{  return GetStringData()->stringLen != 0;  }

    void Clear();
	void Empty()									{  Clear();  }

    char GetAt(size_t n) const						{  CORE_ASSERT(n <= GetStringData()->stringLen);  return m_string[n];  }
    char operator[](int n) const					{  CORE_ASSERT(n <= (int)GetStringData()->stringLen);  return m_string[n];  }
    char operator[](size_t n) const					{  CORE_ASSERT(n <= GetStringData()->stringLen);  return m_string[n];  }
    void SetAt(size_t n, char ch)					{  CORE_ASSERT(n <= GetStringData()->stringLen);  m_string[n] = ch;  }

    operator const char*() const					{  return m_string;  }

	const HeapString& operator+=(const HeapString& right)	{  return ConcatInPlace((int)right.GetStringData()->stringLen, right.m_string);  }
	const HeapString& operator+=(const char* right)	{  return ConcatInPlace((int)StrLen(right), right);  }
	const HeapString& operator+=(char ch)			{  char right[2];  right[0] = ch;  right[1] = 0;  return ConcatInPlace(1, right);  }

    friend HeapString operator+(const HeapString& left, const HeapString& right)
	{
		HeapString s;
		return s.ConcatCopy(left.GetStringData()->stringLen, left.m_string, right.GetStringData()->stringLen, right.m_string);
	}

	friend HeapString operator+(const HeapString& left, const char* right)
	{
		HeapString s;
		return s.ConcatCopy(left.GetStringData()->stringLen, left.m_string, HeapString::StrLen(right), right);
	}

	friend HeapString operator+(const char* left, const HeapString& right)
	{
		HeapString s;
		return s.ConcatCopy(HeapString::StrLen(left), left, right.GetStringData()->stringLen, right.m_string);
	}

	friend HeapString operator+(char ch, const HeapString& right)
	{
		char left[2];
		left[0] = ch;
		left[1] = 0;
		HeapString s;
		return s.ConcatCopy(1, left, right.GetStringData()->stringLen, right.m_string);
	}

	friend HeapString operator+(const HeapString& left, char ch)
	{
		char right[2];
		right[0] = ch;
		right[1] = 0;
		HeapString s;
		return s.ConcatCopy(left.GetStringData()->stringLen, left.m_string, 1, right);
	}

	int Compare(const char* right) const			{ return strcmp(m_string, right); }
	int CompareNoCase(const char* right) const
	{
#if defined(WIN32)
		return _stricmp(m_string, right);
#elif defined(__GNUC__)
		return strcasecmp(m_string, right);
#endif
	}

    HeapString Sub(size_t startPos, size_t count) const;
    HeapString Sub(size_t startPos) const;

    HeapString Upper() const;
    HeapString Lower() const;

	HeapString TrimLeft(const char* trimChars = 0) const;
	HeapString TrimRight(const char* trimChars = 0) const;
	HeapString Trim(const char* trimChars = 0) const;

	int Find(char ch, size_t start = 0) const;
	int Find(const char* strSearch, size_t start = 0) const;
	int ReverseFind(char ch) const;

	int FindOneOf(const char* charSet) const;

	HeapString BeforeLast(char ch) const;
	HeapString AfterLast(char ch) const;

    HeapString Replace(char oldCh, char newCh) const;

    char* GetBuffer(size_t nMinBufLength);
	void ReleaseBuffer();
    void ReleaseBuffer(size_t nNewLength);

	int ToInt() const;
	long ToLong() const;

	HeapStringData* GetStringData() const { return (HeapStringData*)m_string - 1; }

protected:
	void AllocBuffer(size_t nLen);
	void CopyBeforeWrite();
    HeapString& ConcatCopy(size_t src1Len, const char* src1Str, size_t src2Len, const char* src2Str);
	HeapString& ConcatInPlace(int srcStringLen, const char* srcString);
    static size_t StrLen(const char* lpsz)
	{
		return (lpsz == NULL) ? 0 : strlen(lpsz);
	}

	char* m_string;
};

inline bool operator==(const HeapString& left, const HeapString& right)
{  return left.Compare(right) == 0;  }
inline bool operator==(const HeapString& left, const char* right)
{  return left.Compare(right) == 0;  }
inline bool operator==(const char* left, const HeapString& right)
{  return right.Compare(left) == 0;  }
inline bool operator!=(const HeapString& left, const HeapString& right)
{  return left.Compare(right) != 0;  }
inline bool operator!=(const HeapString& left, const char* right)
{  return left.Compare(right) != 0;  }
inline bool operator!=(const char* left, const HeapString& right)
{  return right.Compare(left) != 0;  }
inline bool operator<(const HeapString& left, const HeapString& right)
{  return left.Compare(right) < 0;  }
inline bool operator<(const HeapString& left, const char* right)
{  return left.Compare(right) < 0;  }
inline bool operator<(const char* left, const HeapString& right)
{  return right.Compare(left) > 0;  }
inline bool operator>(const HeapString& left, const HeapString& right)
{  return left.Compare(right) > 0;  }
inline bool operator>(const HeapString& left, const char* right)
{  return left.Compare(right) > 0;  }
inline bool operator>(const char* left, const HeapString& right)
{  return right.Compare(left) < 0;  }
inline bool operator<=(const HeapString& left, const HeapString& right)
{  return left.Compare(right) <= 0;  }
inline bool operator<=(const HeapString& left, const char* right)
{  return left.Compare(right) <= 0;  }
inline bool operator<=(const char* left, const HeapString& right)
{  return right.Compare(left) >= 0;  }
inline bool operator>=(const HeapString& left, const HeapString& right)
{  return left.Compare(right) >= 0;  }
inline bool operator>=(const HeapString& left, const char* right)
{  return left.Compare(right) >= 0;  }
inline bool operator>=(const char* left, const HeapString& right)
{  return right.Compare(left) <= 0;  }


template<> class TypeTraits<HeapString> : public DefaultTypeTraits<HeapString>
{
public:
	static size_t Hash(const HeapString& str)
	{
		size_t l = str.Length();
		size_t h = l;
		const char* ptr = str;
		size_t step = (l >> 5) + 1;
		for (size_t l1 = l; l1 >= step; l1 -= step)
			h = h ^ ((h<<5)+(h>>2)+(unsigned char)(ptr[l1-1]));

		return h;
	}
};

}; // namespace Misc

#endif // MISC_HEAPSTRING_H
