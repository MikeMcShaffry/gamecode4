#include <stdafx.h>
#include "ScintillaPropertiesFile.h"

ScintillaPropertiesFile::ScintillaPropertiesFile()
{
}


ScintillaPropertiesFile::~ScintillaPropertiesFile()
{
}


void ScintillaPropertiesFile::Parse(const CStringA& filename)
{
	CStdioFile file;
	if (!file.Open(CString(filename), CFile::modeRead))
		return;

	bool done = false;
	while (true)
	{
Top:
		if (done)
			break;
		CStringA line;
		bool continuedLine = false;

		while (true)
		{
			CString temp;
			if (!file.ReadString(temp))
			{
				done = true;
				goto Top;
			}
			temp.TrimLeft(' ');
			temp.TrimRight(' ');

			if (temp.IsEmpty())
			{
				if (continuedLine)
					break;
				goto Top;
			}

			if (temp[0] == '#')
			{
				if (continuedLine)
					break;
				goto Top;
			}

			line += temp;

			if (line[line.GetLength() - 1] == '\\')
			{
				// Line continuation.
				continuedLine = true;
			}
			else
				break;
		}
		
		// Resolve all tags.
		int position = -1;
		while (true)
		{
			// Search for $ symbols, denoting a tag.
			position = line.Find('$', position + 1);
			if (position == -1)
				break;
			
			// Okay, there is a tag in there... resolve it.
			if (line[position + 1] == '(')
			{
				int lastpos = line.Find(')', position + 2);
				CStringA tag = line.Mid(position + 2, lastpos - (position + 2));
				
				// See if we can resolve it.  If not, then exit.
				CStringA tagValue;
				BOOL found = m_definitions.Lookup(tag, tagValue);
				if (!found)
				{
					
				}
				
				// Okay, rebuild the string.
				line = line.Left(position) + tagValue +
					line.Right(line.GetLength() - lastpos - 1);
			}
		}
		
		int equalsPos = line.Find('=');
		if (equalsPos == -1)
			continue;

		CStringA key = line.Left(equalsPos);
		CStringA value = line.Mid(equalsPos + 1);

		m_definitions[key] = value;
	}
}


int ScintillaPropertiesFile::GetInt(const CStringA& key, int defaultValue)
{
	CStringA value;
	BOOL found = m_definitions.Lookup(key, value);
	if (!found)
		return defaultValue;
	return atoi(value);
}


CStringA ScintillaPropertiesFile::Get(const CStringA& key)
{
	CStringA value;
	BOOL found = m_definitions.Lookup(key, value);
	return value;	
}


static int IntFromHexDigit(const char ch) {
	if (isdigit(ch))
		return ch - '0';
	else if (ch >= 'A' && ch <= 'F')
		return ch - 'A' + 10;
	else if (ch >= 'a' && ch <= 'f')
		return ch - 'a' + 10;
	else
		return 0;
}

COLORREF ScintillaPropertiesFile::ColourFromString(const char *val) {
	int r = IntFromHexDigit(val[1]) * 16 + IntFromHexDigit(val[2]);
	int g = IntFromHexDigit(val[3]) * 16 + IntFromHexDigit(val[4]);
	int b = IntFromHexDigit(val[5]) * 16 + IntFromHexDigit(val[6]);
	return RGB(r, g, b);
}

StyleDefinition::StyleDefinition(const char *definition) :
size(0), fore(0), back(RGB(0xff,0xff,0xff)), bold(false), italics(false), eolfilled(false), underlined(false) {
	specified = sdNone;
	char *val = strdup(definition);
	//Platform::DebugPrintf("Style %d is [%s]\n", style, val);
	char *opt = val;
	while (opt) {
		char *cpComma = strchr(opt, ',');
		if (cpComma)
			*cpComma = '\0';
		char *colon = strchr(opt, ':');
		if (colon)
			*colon++ = '\0';
		if (0 == strcmp(opt, "italics")) {
			specified = static_cast<flags>(specified | sdItalics);
			italics = true;
		}
		if (0 == strcmp(opt, "notitalics")) {
			specified = static_cast<flags>(specified | sdItalics);
			italics = false;
		}
		if (0 == strcmp(opt, "bold")) {
			specified = static_cast<flags>(specified | sdBold);
			bold = true;
		}
		if (0 == strcmp(opt, "notbold")) {
			specified = static_cast<flags>(specified | sdBold);
			bold = false;
		}
		if (0 == strcmp(opt, "font")) {
			specified = static_cast<flags>(specified | sdFont);
			int colonPos = font.Find(':');
			if (colonPos != -1)
				font = font.Mid(colonPos + 1);
			else
				font.Empty();
		}
		if (0 == strcmp(opt, "fore")) {
			specified = static_cast<flags>(specified | sdFore);
			fore = ScintillaPropertiesFile::ColourFromString(colon);
		}
		if (0 == strcmp(opt, "back")) {
			specified = static_cast<flags>(specified | sdBack);
			back = ScintillaPropertiesFile::ColourFromString(colon);
		}
		if (0 == strcmp(opt, "size")) {
			specified = static_cast<flags>(specified | sdSize);
			size = atoi(colon);
		}
		if (0 == strcmp(opt, "eolfilled")) {
			specified = static_cast<flags>(specified | sdEOLFilled);
			eolfilled = true;
		}
		if (0 == strcmp(opt, "noteolfilled")) {
			specified = static_cast<flags>(specified | sdEOLFilled);
			eolfilled = false;
		}
		if (0 == strcmp(opt, "underlined")) {
			specified = static_cast<flags>(specified | sdUnderlined);
			underlined = true;
		}
		if (0 == strcmp(opt, "notunderlined")) {
			specified = static_cast<flags>(specified | sdUnderlined);
			underlined = false;
		}
		if (cpComma)
			opt = cpComma + 1;
		else
			opt = 0;
	}
	free(val);
}

