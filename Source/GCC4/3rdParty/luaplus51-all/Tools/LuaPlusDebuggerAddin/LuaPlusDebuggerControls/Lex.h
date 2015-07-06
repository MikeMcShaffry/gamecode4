#pragma once

#include <assert.h>

/**
    \internal
	Originally from some article on CodeGuru or Code Project.  Free to use.
	Just revised.  I forget which article.
**/
class LexToken
{
	friend class Lex;
public:
	LexToken() : m_type(NONE) {}
	LexToken(const LexToken& src) : m_type(NONE) {  *this = src;  }
	LexToken& operator=(const LexToken& src);

	~LexToken()
	{
		Destroy();
	}

	void Destroy(void);

	/**
        \internal
	**/
	enum TokenTypes
	{
		NONE = 250,
		ENDOFFILE,
		INTEGER,
		REAL,
		STRING,
		IDENT,
	};

	bool IsEmpty(void) const
	{
		return m_type == NONE;
	}

	bool IsIdent() const
	{
		return m_type == IDENT;
	}

	bool IsString() const
	{
		return m_type == STRING;
	}

	bool IsInteger() const
	{
		return m_type == INTEGER;
	}

	bool IsReal() const
	{
		return m_type == INTEGER || m_type == REAL;
	}

	int GetType(void) const
	{
		return m_type;
	}

	int GetInteger(void) const
	{
		assert(m_type == INTEGER);
		return m_integer;
	}

	float GetReal(void) const
	{
		if (m_type == REAL)
			return m_real;
		else if (m_type == INTEGER)
			return (float)m_integer;
		return 0.0f;
	}

	const CString& GetString(void) const
	{
		assert(m_type == STRING);
		return m_str;
	}

	const CString& GetIdent(void) const
	{
        return m_str;
	}

private:
	union
	{
		int m_integer;
		float m_real;
	};
	CString m_str;

	BYTE m_type;
};


class Lex
{
public:
	Lex() : m_ownFile(false) {}
	~Lex()										{ }
	void Init(const CString& str);
	bool Open(const CString& filename);

	bool Eof() const							{  return m_eof;  }
	int GetCurrentLine(void) const				{  return m_lineNumber;  }
	int GetCurrentColumn(void) const			{  return m_column;  }

	int GetToken(LexToken& token, bool wantSingleChar = false);
	bool UnGetToken(const LexToken& token);
	int GetLine(char* buffer);

	long Seek(LONG position);

protected:
	void ClearToken(void);
	CString LexProcessString(BYTE endChar);
	BYTE LexGetChar(void);
	void LexUnGetChar(BYTE ch);
	int LexReadUndelimited(LexToken& token, bool wantSingleChar = false);

protected:
	BYTE m_savedChar;

	enum { TOKEN_STACK_SIZE = 5 };
	LexToken m_tokenStack[TOKEN_STACK_SIZE];
	int m_tokenStackPos;

	bool m_ownFile;
	bool m_eof;

	int m_lineNumber;
	int m_column;

	int m_internalFileCurPos;
	CString m_internalFileString;
};


inline void Lex::Init(const CString& str)
{
	m_internalFileCurPos = 0;
	m_internalFileString = str;

	m_savedChar = 0;
	m_tokenStackPos = 0;
	m_ownFile = false;
	m_eof = false;
	m_lineNumber = 1;
	m_column = 0;
}


inline void Lex::ClearToken(void)
{
	m_tokenStack[m_tokenStackPos].Destroy();
}


inline BYTE Lex::LexGetChar(void)
{
	if (m_savedChar != 0)
	{
		BYTE tempch = m_savedChar;
		m_savedChar = 0;
		return tempch;
	}

    if (m_internalFileCurPos < m_internalFileString.GetLength())
    {
        return (BYTE)m_internalFileString[m_internalFileCurPos++];
    }
    else
    {
        m_eof = true;
        return (BYTE)EOF;
    }
}


inline void Lex::LexUnGetChar(BYTE ch)
{
	m_savedChar = ch;
	m_column--;
}


inline CString Lex::LexProcessString(BYTE endChar)
{
	BYTE str[1000];
	int len = 0;
	bool done = false;

    while (!done)
    {
        BYTE ch = LexGetChar();
		if (ch == endChar)
			break;

        switch (ch)
        {
            case EOF:
			case '\r':
            case '\n':
                assert(0);
                return _T("");

			case '\\':
				ch = LexGetChar();
				switch (ch)
	   			{
					case 'n':  str[len++] = '\n';  break;
					case 'r':  str[len++] = '\r';  break;
					case 't':  str[len++] = '\t';  break;
					default :
					{
						str[len++] = ch;
					}
				}
				break;

			default:
				str[len++] = ch;
				break;
        }
    }

	str[len] = 0;

	return CString((char*)str);
}


inline int Lex::GetToken(LexToken& token, bool wantSingleChar)
{
	token.Destroy();

	if (m_tokenStackPos > 0)
	{
		token = m_tokenStack[--m_tokenStackPos];
		return token.m_type;
	}

	return LexReadUndelimited(token, wantSingleChar);
}


inline int Lex::LexReadUndelimited(LexToken& token, bool wantSingleChar)
{
	while (true)
	{
		char ch = LexGetChar();

		if (ch == ' '  ||  ch == '\t')
			continue;

		if (ch == EOF)
		{
			token.m_type = LexToken::ENDOFFILE;
			return token.m_type;
		}

		if (ch == '\r')
			continue;

		if (ch == '\n')
		{
            m_lineNumber++;
			m_column = 0;
			continue;
		}

		if (wantSingleChar)
		{
			token.m_type = ch;
			return token.m_type;
		}

		switch (ch)
		{
            case '/':
            	ch = LexGetChar();
            	if (ch != '/')
				{
            	    LexUnGetChar(ch);

					// Create a token.
					token.m_type = '/';
					return token.m_type;
            	}

				while (ch != '\n'  &&  ch != EOF)
					ch = LexGetChar();

				if (ch == '\n')
					LexUnGetChar(ch);

                continue;

			// Process string.
			case '\'':
            case '"':
				// Create a token.
				token.m_type = LexToken::STRING;
				token.m_str = LexProcessString(ch);
				return true;

			case '-':  case '0':  case '1':  case '2':  case '3':
			case '4':  case '5':  case '6':  case '7':  case '8':  case '9':
			{
				if (ch == '-')
				{
					char ch2 = LexGetChar();
					LexUnGetChar(ch2);
					if (ch2 != '.'  &&  (ch2 < '0'  ||  ch2 > '9'))
					{
						token.m_type = '-';
						return token.m_type;
					}
				}

				char numberBuffer[100];
				int numberLen = 0;

				numberBuffer[numberLen++] = ch;

				// Test for a hex number.
				bool isFloatNumber = false;
				while (1)
				{
					ch = LexGetChar();
					if (isdigit(ch)  ||  ch == '.')
					{
						numberBuffer[numberLen++] = ch;
					}
					else if (ch == 'e')
					{
						ch = LexGetChar();
						if (!isdigit(ch)  &&  ch != '-' &&  ch != '+')
						{
							LexUnGetChar(ch);
							break;
						}

						numberBuffer[numberLen++] = 'e';
						numberBuffer[numberLen++] = ch;
						isFloatNumber = true;
					}
					else
					{
						LexUnGetChar(ch);
						break;
					}

					if (ch == '.')
						isFloatNumber = true;
				}

				numberBuffer[numberLen] = 0;
				if (isFloatNumber == false)
				{
					token.m_type = LexToken::INTEGER;
					token.m_integer = atoi(numberBuffer);
					return token.m_type;
				}
				else
				{
					token.m_type = LexToken::REAL;
					token.m_real = (float)atof(numberBuffer);
					return token.m_type;
				}

				break;
			}

			default:
			{
				if (ch != '_'  &&  !isalpha(ch))
				{
					token.m_type = ch;
					return token.m_type;
				}

				char identBuffer[1024];
				int identLen = 0;

				identBuffer[identLen++] = ch;

				while (1)
				{
					ch = LexGetChar();
					if (isalnum(ch)  ||  ch == '_')
						identBuffer[identLen++] = ch;
					else
					{
						LexUnGetChar(ch);
						break;
					}
				}

				identBuffer[identLen] = 0;

				token.m_type = LexToken::IDENT;

				token.m_str = identBuffer;
				return token.m_type;
			}

		}
	}

	return LexToken::NONE;
}


inline bool Lex::UnGetToken(const LexToken& token)
{
	m_tokenStack[m_tokenStackPos++] = token;

	return true;
}


inline int Lex::GetLine(char* buffer)
{
	int pos = 0;
	char ch = 0;

	bool done = false;
	while (!done)
	{
		ch = LexGetChar();
		switch (ch)
		{
			case EOF:
				buffer[pos] = 0;
				done = true;
				break;

			case '\r':
			case '\n':
				buffer[pos] = 0;

				if (ch == '\r')
				{
					ch = LexGetChar();
					if (ch != '\n')
						LexUnGetChar(ch);
				}

				m_lineNumber++;
				done = true;
				break;

			default:
				buffer[pos++] = ch;
				break;
		}
	}

	return ch;
}


inline long Lex::Seek(LONG position)
{
	m_tokenStackPos = 0;
	return 0;
}


//=============================================================================
// LexToken

inline LexToken& LexToken::operator=(const LexToken& src)
{
	Destroy();

	m_type = src.m_type;
	switch (src.m_type)
	{
		case INTEGER:
			m_integer = src.m_integer;
			break;
		case REAL:
			m_real = src.m_real;
			break;
		case STRING:
		case IDENT:
			m_str = src.m_str;
			break;
	}

	return *this;
}

inline void LexToken::Destroy()
{
	if (m_type == STRING  ||  m_type == IDENT)
	{
		m_str.Empty();
	}
	m_type = NONE;
}


