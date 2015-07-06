#pragma once

#include "scintilla.h"

class ScintillaPropertiesFile;

class ScintillaEditor
{
public:
	ScintillaEditor();
	~ScintillaEditor();

	void Create(HWND parent, const CRect& editorRect);
	void Destroy();

	HWND GetWindowHandle()					{  return m_editorWnd;  }
	
	bool OpenFile(const CString& filename);
	bool OpenFile(const CString& filename, const CString& forcedExt);
	bool SaveFile(const CString& fileName);
	void GetRange(int start, int end, char* text);
	DWORD GetSelText(char* text);
	void CenterPosition(int position);

	CharacterRange GetSelection();
	int GetDocumentLength();
	void SetSelection(int anchor, int currentPos)	{  SendEditor(SCI_SETSEL, anchor, currentPos);  }
	void EnsureRangeVisible(int posStart, int posEnd);

	void GotoLine(int line, bool ensureVisible);

	int SendEditor(UINT msg, WPARAM wParam = 0, LPARAM lParam = 0);
	int SendEditorString(UINT msg, WPARAM wParam, const char* s)
	{
		return SendEditor(msg, wParam, reinterpret_cast<LPARAM>(s));
	}

	DWORD GetCurrentPos()							{  return SendEditor(SCI_GETCURRENTPOS, 0, 0);  }
	DWORD PositionFromLine(int line)				{  return SendEditor(SCI_POSITIONFROMLINE, line, 0);  }
	DWORD GetLineEndPosition(int line)				{  return SendEditor(SCI_GETLINEENDPOSITION, line, 0);  }

	CString GetWord();

//	int Call(UINT msg, WPARAM wParam = 0, LPARAM lParam = 0);

protected:
	void SetStyleFor(const char *lang);
	void SetOneStyle(int style, const char *s);
	void ForwardPropertyToEditor(const char *key);
	void ReadPropertiesInitial();

	bool ReadProperties(const CString& filename, const CString& forcedExt);


private:
	static HMODULE s_editorModule;

	HWND m_editorWnd;

	int (*m_editorCall)(void*,int,int,int);
	void* m_editorCallPtr;

	bool margin;
	int marginWidth;
	enum { marginWidthDefault = 20};

	bool foldMargin;
	int foldMarginWidth;
	enum { foldMarginWidthDefault = 14};

	bool lineNumbers;
	int lineNumbersWidth;
	enum { lineNumbersWidthDefault = 40};

	ScintillaPropertiesFile* m_props;

};


