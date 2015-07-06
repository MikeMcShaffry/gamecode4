#pragma once

#include <afxtempl.h>

class ScintillaPropertiesFile
{
public:
	ScintillaPropertiesFile();
	~ScintillaPropertiesFile();

	void Parse(const CStringA& filename);
	int GetInt(const CStringA& key, int defaultValue = 0);
	CStringA Get(const CStringA& key);
	static COLORREF ColourFromString(const char *val);
		
protected:
	CMap<CStringA, LPCSTR, CStringA, const CStringA&> m_definitions;
};


class StyleDefinition {
public:
	CStringA font;
	int size;
	COLORREF fore;
	COLORREF back;
	bool bold;
	bool italics;
	bool eolfilled;
	bool underlined;
	enum flags { sdNone = 0, sdFont = 0x1, sdSize = 0x2, sdFore = 0x4, sdBack = 0x8,
	       sdBold = 0x10, sdItalics = 0x20, sdEOLFilled = 0x40, sdUnderlined=0x80 } specified;
	StyleDefinition(const char *definition);
};

