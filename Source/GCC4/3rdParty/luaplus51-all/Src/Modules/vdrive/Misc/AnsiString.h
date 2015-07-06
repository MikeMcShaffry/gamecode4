#ifndef MISC_ANSISTRING_H
#define MISC_ANSISTRING_H

#include "Misc.h"
#include "TypeTraits.h"
#include <memory.h>
#include <string.h>
#include <stdarg.h>

namespace Misc {

struct AnsiStringData
{
    int refCount;
    size_t stringLen;
	size_t maxSize;

    char* data() const   { return (char*)(this+1); }

    bool  IsShared() const { return (refCount > 1);   }

    void  Ref()										{  if (refCount != -1) refCount++;  }
    void  Unref()									{  if (refCount != -1  &&  --refCount == 0)  Free();  }
    void  Free();
};


extern const char* anEmptyAnsiString;


class AnsiString
{
public:
	static AnsiString Format(const char* format, ...);
	static AnsiString FormatV(const char* format, va_list args);

	AnsiString()										{  m_string = (char*)anEmptyAnsiString;  }
    AnsiString(const AnsiString& stringSrc)
    {
        if ( stringSrc.IsEmpty() )
        {
            m_string = (char*)anEmptyAnsiString;
        }
        else
        {
            m_string = stringSrc.m_string;            // share same data
            GetStringData()->Ref();                    // => one more copy
        }
    }

    AnsiString(size_t n, char ch)
	{
		m_string = (char*)anEmptyAnsiString;
		if (n >= 1)
		{
			AllocBuffer(n);
			for (size_t i = 0; i < n; ++i)
				m_string[i] = ch;
		}
	}

	AnsiString(const char* str)
    {
        m_string = (char*)anEmptyAnsiString;
        size_t len = StrLen(str);
        if (len > 0)
        {
            AllocBuffer(len);
            memcpy(m_string, str, len);
        }
    }

	AnsiString(const char* str, size_t len)
    {
		m_string = (char*)anEmptyAnsiString;
		if (len == (size_t)-1)
			len = StrLen(str);
		if (len > 0)
		{
			AllocBuffer(len);
			memcpy(m_string, str, len);
		}
    }

	AnsiString(char ch)
	{
		m_string = (char*)anEmptyAnsiString;
		AllocBuffer(1);
		m_string[0] = ch;
		m_string[1] = 0;
	}

	explicit AnsiString(const wchar* str)
    {
        m_string = (char*)anEmptyAnsiString;
        size_t len = 0;
		while (str[len] != 0) len++;
        if (len > 0)
        {
            AllocBuffer(len);
			for (size_t i = 0; i < len; ++i)
				m_string[i] = (char)(BYTE)str[i];
        }
    }

	~AnsiString()
    {
        GetStringData()->Unref();
    }

    const AnsiString& operator=(const AnsiString& stringSrc);
    const AnsiString& operator=(const char* srcString);
    const AnsiString& operator=(char ch);

    size_t Length() const							{  return GetStringData()->stringLen;  }
    bool IsEmpty() const							{  return GetStringData()->stringLen == 0;  }
    bool IsNotEmpty() const							{  return GetStringData()->stringLen != 0;  }

    void Clear();
	void Empty()									{  Clear();  }

    char GetAt(size_t n) const						{  CORE_ASSERT(n <= GetStringData()->stringLen);  return m_string[n];  }
    char operator[](int n) const					{  CORE_ASSERT(n <= (int)GetStringData()->stringLen);  return m_string[n];  }
    char operator[](size_t n) const					{  CORE_ASSERT(n <= GetStringData()->stringLen);  return m_string[n];  }
    void SetAt(size_t n, char ch)					{  CORE_ASSERT(n <= GetStringData()->stringLen);  CopyBeforeWrite();  m_string[n] = ch;  }

    operator const char*() const					{  return m_string;  }

	const AnsiString& operator+=(const AnsiString& right)	{  return ConcatInPlace((int)right.GetStringData()->stringLen, right.m_string);  }
	const AnsiString& operator+=(const char* right)	{  return ConcatInPlace((int)StrLen(right), right);  }
	const AnsiString& operator+=(char ch)			{  char right[2];  right[0] = ch;  right[1] = 0;  return ConcatInPlace(1, right);  }

    friend AnsiString operator+(const AnsiString& left, const AnsiString& right)
	{
		AnsiString s;
		return s.ConcatCopy(left.GetStringData()->stringLen, left.m_string, right.GetStringData()->stringLen, right.m_string);
	}

	friend AnsiString operator+(const AnsiString& left, const char* right)
	{
		AnsiString s;
		return s.ConcatCopy(left.GetStringData()->stringLen, left.m_string, AnsiString::StrLen(right), right);
	}

	friend AnsiString operator+(const char* left, const AnsiString& right)
	{
		AnsiString s;
		return s.ConcatCopy(AnsiString::StrLen(left), left, right.GetStringData()->stringLen, right.m_string);
	}

	friend AnsiString operator+(char ch, const AnsiString& right)
	{
		char left[2];
		left[0] = ch;
		left[1] = 0;
		AnsiString s;
		return s.ConcatCopy(1, left, right.GetStringData()->stringLen, right.m_string);
	}

	friend AnsiString operator+(const AnsiString& left, char ch)
	{
		char right[2];
		right[0] = ch;
		right[1] = 0;
		AnsiString s;
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

    AnsiString Sub(size_t startPos, size_t count) const;
    AnsiString Sub(size_t startPos) const;

    AnsiString Upper() const;
    AnsiString Lower() const;

	AnsiString TrimLeft(const char* trimChars = 0) const;
	AnsiString TrimRight(const char* trimChars = 0) const;
	AnsiString Trim(const char* trimChars = 0) const;

	int Find(char ch, size_t start = 0) const;
	int Find(const char* strSearch, size_t start = 0) const;
	int ReverseFind(char ch) const;

	int FindOneOf(const char* charSet) const;

	AnsiString BeforeLast(char ch) const;
	AnsiString AfterLast(char ch) const;

    AnsiString Replace(char oldCh, char newCh) const;

    char* GetBuffer(size_t nMinBufLength);
	void ReleaseBuffer();
    void ReleaseBuffer(size_t nNewLength);

	int ToInt() const;
	long ToLong() const;

	AnsiStringData* GetStringData() const { return (AnsiStringData*)m_string - 1; }

protected:
	void AllocBuffer(size_t nLen);
	void CopyBeforeWrite();
	void AllocBeforeWrite(size_t);
    AnsiString& ConcatCopy(size_t src1Len, const char* src1Str, size_t src2Len, const char* src2Str);
	AnsiString& ConcatInPlace(int srcStringLen, const char* srcString);
    static size_t StrLen(const char* lpsz)
	{
		return (lpsz == NULL) ? 0 : strlen(lpsz);
	}

	char* m_string;
};

inline bool operator==(const AnsiString& left, const AnsiString& right)
{  return left.Compare(right) == 0;  }
inline bool operator==(const AnsiString& left, const char* right)
{  return left.Compare(right) == 0;  }
inline bool operator==(const char* left, const AnsiString& right)
{  return right.Compare(left) == 0;  }
inline bool operator!=(const AnsiString& left, const AnsiString& right)
{  return left.Compare(right) != 0;  }
inline bool operator!=(const AnsiString& left, const char* right)
{  return left.Compare(right) != 0;  }
inline bool operator!=(const char* left, const AnsiString& right)
{  return right.Compare(left) != 0;  }
inline bool operator<(const AnsiString& left, const AnsiString& right)
{  return left.Compare(right) < 0;  }
inline bool operator<(const AnsiString& left, const char* right)
{  return left.Compare(right) < 0;  }
inline bool operator<(const char* left, const AnsiString& right)
{  return right.Compare(left) > 0;  }
inline bool operator>(const AnsiString& left, const AnsiString& right)
{  return left.Compare(right) > 0;  }
inline bool operator>(const AnsiString& left, const char* right)
{  return left.Compare(right) > 0;  }
inline bool operator>(const char* left, const AnsiString& right)
{  return right.Compare(left) < 0;  }
inline bool operator<=(const AnsiString& left, const AnsiString& right)
{  return left.Compare(right) <= 0;  }
inline bool operator<=(const AnsiString& left, const char* right)
{  return left.Compare(right) <= 0;  }
inline bool operator<=(const char* left, const AnsiString& right)
{  return right.Compare(left) >= 0;  }
inline bool operator>=(const AnsiString& left, const AnsiString& right)
{  return left.Compare(right) >= 0;  }
inline bool operator>=(const AnsiString& left, const char* right)
{  return left.Compare(right) >= 0;  }
inline bool operator>=(const char* left, const AnsiString& right)
{  return right.Compare(left) <= 0;  }


template<> class TypeTraits<AnsiString> : public DefaultTypeTraits<AnsiString>
{
public:
	static size_t Hash(const AnsiString& str)
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


bool StringToInt(const AnsiString& theString, int* theIntVal);
bool StringToDouble(const AnsiString& theString, double* theDoubleVal);

}; // namespace Misc

#endif // MISC_ANSISTRING_H
