#include <StdAfx.h>
#include "ScintillaEditor.h"
#include "ScintillaPropertiesFile.h"
#include "scilexer.h"

HMODULE ScintillaEditor::s_editorModule = NULL;

ScintillaEditor::ScintillaEditor() :
	m_editorWnd(NULL),
	m_editorCall(NULL),
	m_editorCallPtr(NULL),
	m_props(NULL)
{
}


ScintillaEditor::~ScintillaEditor()
{
	Destroy();

	delete m_props;
}


void ScintillaEditor::Create(HWND parent, const CRect& editorRect)
{
	if (!s_editorModule)
	{
		s_editorModule = LoadLibrary(_T("SciLexer.DLL"));
		if (s_editorModule == NULL)
		{
			::MessageBox(NULL, _T("The Scintilla DLL could not be loaded."),
				_T("Error loading Scintilla"),
				MB_OK | MB_ICONERROR);
		}
	}

	m_editorWnd = CreateWindowEx(0, _T("Scintilla"), _T(""),
			WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			editorRect.left, editorRect.top, editorRect.Width(), editorRect.Height(),
			parent, NULL, NULL, NULL);

	m_editorCall = (int (__cdecl *)(void *,int,int,int))::SendMessage(m_editorWnd, SCI_GETDIRECTFUNCTION, 0, 0);
	m_editorCallPtr = (void *)::SendMessage(m_editorWnd, SCI_GETDIRECTPOINTER, 0, 0);
}


void ScintillaEditor::Destroy()
{
	if (m_editorWnd)
	{
		::SendMessage(m_editorWnd, WM_DESTROY, 0, 0);
		m_editorWnd = NULL;
		m_editorCall = NULL;
		m_editorCallPtr = NULL;
	}

	delete m_props;
	m_props = NULL;
}


int ScintillaEditor::SendEditor(UINT msg, WPARAM wParam, LPARAM lParam)
{
	return m_editorCall(m_editorCallPtr, msg, wParam, lParam);
}



void ScintillaEditor::SetOneStyle(int style, const char *s)
{
	StyleDefinition sd(s);
	if (sd.specified & StyleDefinition::sdItalics)
		SendEditor(SCI_STYLESETITALIC, style, sd.italics ? 1 : 0);
	if (sd.specified & StyleDefinition::sdBold)
		SendEditor(SCI_STYLESETBOLD, style, sd.bold ? 1 : 0);
	if (sd.specified & StyleDefinition::sdFont)
		SendEditor(SCI_STYLESETFONT, style, reinterpret_cast<long>((LPCSTR)sd.font));
	if (sd.specified & StyleDefinition::sdFore)
		SendEditor(SCI_STYLESETFORE, style, sd.fore);
	if (sd.specified & StyleDefinition::sdBack)
		SendEditor(SCI_STYLESETBACK, style, sd.back);
	if (sd.specified & StyleDefinition::sdSize)
		SendEditor(SCI_STYLESETSIZE, style, sd.size);
	if (sd.specified & StyleDefinition::sdEOLFilled)
		SendEditor(SCI_STYLESETEOLFILLED, style, sd.eolfilled ? 1 : 0);
	if (sd.specified & StyleDefinition::sdUnderlined)
		SendEditor(SCI_STYLESETUNDERLINE, style, sd.underlined ? 1 : 0);
//	fn(ptr, SCI_STYLESETCHARACTERSET, style, characterSet);
}


int value(CStringA& s) {
	return atoi(s);
}


void ScintillaEditor::ForwardPropertyToEditor(const char *key)
{
	CStringA value = m_props->Get(key);
	SendEditorString(SCI_SETPROPERTY, (WPARAM)(key), value);
}

void ScintillaEditor::ReadPropertiesInitial()
{
	ScintillaPropertiesFile& props = *m_props;

	int indentationWSVisible = props.GetInt("view.indentation.whitespace", 1);
//	ViewWhitespace(props.GetInt("view.whitespace"));
	SendEditor(SCI_SETINDENTATIONGUIDES, props.GetInt("view.indentation.guides"));
	SendEditor(SCI_SETVIEWEOL, props.GetInt("view.eol"));

//	sbVisible = props.GetInt("statusbar.visible");
//	tbVisible = props.GetInt("toolbar.visible");
//	tabVisible = props.GetInt("tabbar.visible");
//	tabMultiLine = props.GetInt("tabbar.multiline");
#if PLAT_WIN
//	if (tabMultiLine) {	// Windows specific!
//		long wl = ::GetWindowLong(wTabBar.GetID(), GWL_STYLE);
//		::SetWindowLong(wTabBar.GetID(), GWL_STYLE, wl | TCS_MULTILINE);
//	}
#endif

	lineNumbersWidth = 0;
	CStringA linenums = props.Get("line.numbers");
	if (linenums.GetLength())
		lineNumbersWidth = value(linenums);
	int lineNumbers = lineNumbersWidth;
	if (lineNumbersWidth == 0)
		lineNumbersWidth = lineNumbersWidthDefault;

	marginWidth = 0;
	CStringA margwidth = props.Get("margin.width");
	if (margwidth.GetLength())
		marginWidth = value(margwidth);
	margin = marginWidth != 0;
	if (marginWidth == 0)
		marginWidth = marginWidthDefault;

	foldMarginWidth = props.GetInt("fold.margin.width", foldMarginWidthDefault);
	foldMargin = foldMarginWidth != 0;
	if (foldMarginWidth == 0)
		foldMarginWidth = foldMarginWidthDefault;

/*	char homepath[MAX_PATH + 20];
	if (GetSciteDefaultHome(homepath, sizeof(homepath))) {
		props.Set("SciteDefaultHome", homepath);
	}
	if (GetSciteUserHome(homepath, sizeof(homepath))) {
		props.Set("SciteUserHome", homepath);
	}
*/
}


bool ScintillaEditor::ReadProperties(const CString& fileName, const CString& forcedExt)
{
	delete m_props;
	m_props = NULL;

	CStringA fileNameForExtension;
	if (forcedExt.IsEmpty())
	{
		int dotPos = fileName.ReverseFind('.');
		if (dotPos != -1)
		{
			fileNameForExtension = fileName.Mid(dotPos + 1);
			fileNameForExtension.MakeLower();
		}
	}
	else
		fileNameForExtension = forcedExt;

	TCHAR moduleFileNameBuffer[_MAX_PATH];
	GetModuleFileName(GetModuleHandle(NULL), moduleFileNameBuffer, _MAX_PATH);
	CStringA modulePath = moduleFileNameBuffer;
	int slashPos = modulePath.ReverseFind('\\');
	modulePath = modulePath.Left(slashPos + 1);

	m_props = new ScintillaPropertiesFile;


	m_props->Parse(modulePath + "SciTEGlobal.properties");
	m_props->Parse(modulePath + fileNameForExtension + ".properties");

	ScintillaPropertiesFile& props = *m_props;

	const CStringA& language = props.Get("lexer.*." + fileNameForExtension);

	int lexLanguage;
	if (language == "python") {
		lexLanguage = SCLEX_PYTHON;
	} else if (language == "cpp") {
		lexLanguage = SCLEX_CPP;
	} else if (language == "hypertext") {
		lexLanguage = SCLEX_HTML;
	} else if (language == "xml") {
		lexLanguage = SCLEX_XML;
	} else if (language == "perl") {
		lexLanguage = SCLEX_PERL;
	} else if (language == "sql") {
		lexLanguage = SCLEX_SQL;
	} else if (language == "vb") {
		lexLanguage = SCLEX_VB;
	} else if (language == "props") {
		lexLanguage = SCLEX_PROPERTIES;
	} else if (language == "errorlist") {
		lexLanguage = SCLEX_ERRORLIST;
	} else if (language == "makefile") {
		lexLanguage = SCLEX_MAKEFILE;
	} else if (language == "batch") {
		lexLanguage = SCLEX_BATCH;
	} else if (language == "latex") {
		lexLanguage = SCLEX_LATEX;
	} else if (language == "lua") {
		lexLanguage = SCLEX_LUA;
	} else if (language == "diff") {
		lexLanguage = SCLEX_DIFF;
	} else if (language == "container") {
		lexLanguage = SCLEX_CONTAINER;
	} else if (language == "conf") {
		lexLanguage = SCLEX_CONF;
	} else if (language == "pascal") {
		lexLanguage = SCLEX_PASCAL;
	} else if (language == "ave") {
		lexLanguage = SCLEX_AVE;
	} else {
		lexLanguage = SCLEX_NULL;
	}

	if ((lexLanguage == SCLEX_HTML) || (lexLanguage == SCLEX_XML))
		SendEditor(SCI_SETSTYLEBITS, 7);
	else
		SendEditor(SCI_SETSTYLEBITS, 5);

	SendEditor(SCI_SETLEXER, lexLanguage);
//	SendOutput(SCI_SETLEXER, SCLEX_ERRORLIST);

//	apis.Clear();

	CStringA kw0 = props.Get("keywords.*." + fileNameForExtension);
	SendEditorString(SCI_SETKEYWORDS, 0, kw0);
	CStringA kw1 = props.Get("keywords2.*." + fileNameForExtension);
	SendEditorString(SCI_SETKEYWORDS, 1, kw1);
	CStringA kw2 = props.Get("keywords3.*." + fileNameForExtension);
	SendEditorString(SCI_SETKEYWORDS, 2, kw2);
	CStringA kw3 = props.Get("keywords4.*." + fileNameForExtension);
	SendEditorString(SCI_SETKEYWORDS, 3, kw3);
	CStringA kw4 = props.Get("keywords5.*." + fileNameForExtension);
	SendEditorString(SCI_SETKEYWORDS, 4, kw4);

//	char homepath[MAX_PATH + 20];
//	if (GetSciteDefaultHome(homepath, sizeof(homepath))) {
//		props.Set("SciteDefaultHome", homepath);
//	}
//	if (GetSciteUserHome(homepath, sizeof(homepath))) {
//		props.Set("SciteUserHome", homepath);
//	}

	ForwardPropertyToEditor("fold");
	ForwardPropertyToEditor("fold.compact");
	ForwardPropertyToEditor("fold.comment");
	ForwardPropertyToEditor("fold.html");
	ForwardPropertyToEditor("styling.within.preprocessor");
	ForwardPropertyToEditor("tab.timmy.whinge.level");

/*	CStringA apifilename = props.Get("api.", fileNameForExtension);
	if (apifilename.GetLength()) {
		FILE *fp = fopen(apifilename, fileRead);
		if (fp) {
			fseek(fp, 0, SEEK_END);
			int len = ftell(fp);
			char *buffer = apis.Allocate(len);
			if (buffer) {
				fseek(fp, 0, SEEK_SET);
				fread(buffer, 1, len, fp);
				apis.SetFromAllocated();
			}
			fclose(fp);
			//Platform::DebugPrintf("Finished api file %d\n", len);
		}

	}
*/
	if (!props.GetInt("eol.auto")) {
		CStringA eol_mode = props.Get("eol.mode");
		if (eol_mode == "LF") {
			SendEditor(SCI_SETEOLMODE, SC_EOL_LF);
		} else if (eol_mode == "CR") {
			SendEditor(SCI_SETEOLMODE, SC_EOL_CR);
		} else if (eol_mode == "CRLF") {
			SendEditor(SCI_SETEOLMODE, SC_EOL_CRLF);
		}
	}

//	codePage = props.GetInt("code.page");
//	SendEditor(SCI_SETCODEPAGE, codePage);

//	characterSet = props.GetInt("character.set");

	CStringA colour;
	colour = props.Get("caret.fore");
	if (colour.GetLength()) {
		SendEditor(SCI_SETCARETFORE, ScintillaPropertiesFile::ColourFromString(colour));
	}

	SendEditor(SCI_SETCARETWIDTH, props.GetInt("caret.width", 1));
//	SendOutput(SCI_SETCARETWIDTH, props.GetInt("caret.width", 1));

	colour = props.Get("calltip.back");
	if (colour.GetLength()) {
		SendEditor(SCI_CALLTIPSETBACK, ScintillaPropertiesFile::ColourFromString(colour));
	}

	CStringA caretPeriod = props.Get("caret.period");
	if (caretPeriod.GetLength()) {
		SendEditor(SCI_SETCARETPERIOD, value(caretPeriod));
//		SendOutput(SCI_SETCARETPERIOD, caretPeriod.value());
	}

	int caretStrict = props.GetInt("caret.policy.strict") ? CARET_STRICT : 0;
	int caretSlop = props.GetInt("caret.policy.slop", 1) ? CARET_SLOP : 0;
	int caretLines = props.GetInt("caret.policy.lines");
	SendEditor(SCI_SETCARETPOLICY, caretStrict | caretSlop, caretLines);

	int visibleStrict = props.GetInt("visible.policy.strict") ? VISIBLE_STRICT : 0;
	int visibleSlop = props.GetInt("visible.policy.slop", 1) ? VISIBLE_SLOP : 0;
	int visibleLines = props.GetInt("visible.policy.lines");
	SendEditor(SCI_SETVISIBLEPOLICY, visibleStrict | visibleSlop, visibleLines);

	SendEditor(SCI_SETEDGECOLUMN, props.GetInt("edge.column", 0));
	SendEditor(SCI_SETEDGEMODE, props.GetInt("edge.mode", EDGE_NONE));
	colour = props.Get("edge.colour");
	if (colour.GetLength()) {
		SendEditor(SCI_SETEDGECOLOUR, ScintillaPropertiesFile::ColourFromString(colour));
	}

	CStringA selfore = props.Get("selection.fore");
	if (selfore.GetLength()) {
		SendEditor(SCI_SETSELFORE, 1, ScintillaPropertiesFile::ColourFromString(selfore));
	} else {
		SendEditor(SCI_SETSELFORE, 0, 0);
	}
	colour = props.Get("selection.back");
	if (colour.GetLength()) {
		SendEditor(SCI_SETSELBACK, 1, ScintillaPropertiesFile::ColourFromString(colour));
	} else {
		if (selfore.GetLength())
			SendEditor(SCI_SETSELBACK, 0, 0);
		else	// Have to show selection somehow
			SendEditor(SCI_SETSELBACK, 1, RGB(0xC0, 0xC0, 0xC0));
	}

	char bracesStyleKey[200];
	sprintf(bracesStyleKey, "braces.%s.style", language);
	int bracesStyle = props.GetInt(bracesStyleKey, 0);

	char key[200];
	CStringA sval;

	sprintf(key, "calltip.%s.ignorecase", "*");
	sval = props.Get(key);
	bool callTipIgnoreCase = sval == "1";
	sprintf(key, "calltip.%s.ignorecase", language);
	sval = props.Get(key);
	if (sval != "")
		callTipIgnoreCase = sval == "1";

	sprintf(key, "autocomplete.%s.ignorecase", "*");
	sval = props.Get(key);
	bool autoCompleteIgnoreCase = sval == "1";
	sprintf(key, "autocomplete.%s.ignorecase", language);
	sval = props.Get(key);
	if (sval != "")
		autoCompleteIgnoreCase = sval == "1";
	SendEditor(SCI_AUTOCSETIGNORECASE, autoCompleteIgnoreCase ? 1 : 0);

	int autoCChooseSingle = props.GetInt("autocomplete.choose.single");
	SendEditor(SCI_AUTOCSETCHOOSESINGLE, autoCChooseSingle),

	// Set styles
	// For each window set the global default style, then the language default style, then the other global styles, then the other language styles

	SendEditor(SCI_STYLERESETDEFAULT, 0, 0);
//	SendOutput(SCI_STYLERESETDEFAULT, 0, 0);

	sprintf(key, "style.%s.%0d", "*", STYLE_DEFAULT);
	sval = props.Get(key);
	SetOneStyle(STYLE_DEFAULT, sval);
//	SetOneStyle(wOutput, STYLE_DEFAULT, sval);

	sprintf(key, "style.%s.%0d", language, STYLE_DEFAULT);
	sval = props.Get(key);
	SetOneStyle(STYLE_DEFAULT, sval);

	SendEditor(SCI_STYLECLEARALL, 0, 0);

	SetStyleFor("*");
	SetStyleFor(language);

//	SendOutput(SCI_STYLECLEARALL, 0, 0);

//	sprintf(key, "style.%s.%0d", "errorlist", STYLE_DEFAULT);
//	sval = props.Get(key, "");
//	SetOneStyle(wOutput, STYLE_DEFAULT, sval);

//	SendOutput(SCI_STYLECLEARALL, 0, 0);

//	SetStyleFor(wOutput, "*");
//	SetStyleFor(wOutput, "errorlist");

//	if (firstPropertiesRead) {
		ReadPropertiesInitial();
//	}

	SendEditor(SCI_SETUSEPALETTE, props.GetInt("use.palette"));
	SendEditor(SCI_SETPRINTMAGNIFICATION, props.GetInt("print.magnification"));
	SendEditor(SCI_SETPRINTCOLOURMODE, props.GetInt("print.colour.mode"));

	int clearBeforeExecute = props.GetInt("clear.before.execute");

	int blankMarginLeft = props.GetInt("blank.margin.left", 1);
	int blankMarginRight = props.GetInt("blank.margin.right", 1);
	//long marginCombined = Platform::LongFromTwoShorts(blankMarginLeft, blankMarginRight);
	SendEditor(SCI_SETMARGINLEFT, 0, blankMarginLeft);
	SendEditor(SCI_SETMARGINRIGHT, 0, blankMarginRight);
//	SendOutput(SCI_SETMARGINLEFT, 0, blankMarginLeft);
//	SendOutput(SCI_SETMARGINRIGHT, 0, blankMarginRight);

	SendEditor(SCI_SETMARGINWIDTHN, 1, margin ? marginWidth : 0);
//	SendEditor(SCI_SETMARGINWIDTHN, 0, lineNumbers ? lineNumbersWidth : 0);

	int bufferedDraw = props.GetInt("buffered.draw", 1);
	SendEditor(SCI_SETBUFFEREDDRAW, bufferedDraw);

	int bracesCheck = props.GetInt("braces.check");
	int bracesSloppy = props.GetInt("braces.sloppy");

	CStringA wordCharacters = props.Get("word.characters." + fileNameForExtension);
	if (wordCharacters.GetLength()) {
		SendEditorString(SCI_SETWORDCHARS, 0, wordCharacters);
	} else {
		SendEditor(SCI_SETWORDCHARS, 0, 0);
	}

	SendEditor(SCI_SETUSETABS, props.GetInt("use.tabs", 1));
	int tabSize = props.GetInt("tabsize");
	if (tabSize) {
		SendEditor(SCI_SETTABWIDTH, tabSize);
	}
	int indentSize = props.GetInt("indent.size");
	SendEditor(SCI_SETINDENT, indentSize);
	int indentOpening = props.GetInt("indent.opening");
	int indentClosing = props.GetInt("indent.closing");
	CStringA lookback = props.Get("statement.lookback." + fileNameForExtension);
	int statementLookback = value(lookback);
/*	statementIndent = GetStyleAndWords("statement.indent.");
	statementEnd = GetStyleAndWords("statement.end.");
	blockStart = GetStyleAndWords("block.start.");
	blockEnd = GetStyleAndWords("block.end.");
*/
/*	if (props.GetInt("vc.home.key", 1)) {
		AssignKey(SCK_HOME, 0, SCI_VCHOME);
		AssignKey(SCK_HOME, SCMOD_SHIFT, SCI_VCHOMEEXTEND);
	} else {
		AssignKey(SCK_HOME, 0, SCI_HOME);
		AssignKey(SCK_HOME, SCMOD_SHIFT, SCI_HOMEEXTEND);
	}
	AssignKey('L', SCMOD_SHIFT | SCMOD_CTRL, SCI_LINEDELETE);
*/
	SendEditor(SCI_SETHSCROLLBAR, props.GetInt("horizontal.scrollbar", 1));
//	SendOutput(SCI_SETHSCROLLBAR, props.GetInt("output.horizontal.scrollbar", 1));

	int tabHideOne = props.GetInt("tabbar.hide.one");

//	SetToolsMenu();

	SendEditor(SCI_SETFOLDFLAGS, props.GetInt("fold.flags"));

	// To put the folder markers in the line number region
	//SendEditor(SCI_SETMARGINMASKN, 0, SC_MASK_FOLDERS);

	SendEditor(SCI_SETMODEVENTMASK, SC_MOD_CHANGEFOLD);

	// Create a margin column for the folding symbols
	SendEditor(SCI_SETMARGINTYPEN, 2, SC_MARGIN_SYMBOL);

	SendEditor(SCI_SETMARGINWIDTHN, 2, foldMargin ? foldMarginWidth : 0);

	SendEditor(SCI_SETMARGINMASKN, 2, SC_MASK_FOLDERS);
	SendEditor(SCI_SETMARGINSENSITIVEN, 2, 1);

	if (props.GetInt("fold.use.plus")) {
		SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_MINUS);
		SendEditor(SCI_MARKERSETFORE, SC_MARKNUM_FOLDEROPEN, RGB(0xff, 0xff, 0xff));
		SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDEROPEN, RGB(0, 0, 0));
		SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_PLUS);
		SendEditor(SCI_MARKERSETFORE, SC_MARKNUM_FOLDER, RGB(0xff, 0xff, 0xff));
		SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDER, RGB(0, 0, 0));
	} else {
		SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_ARROWDOWN);
		SendEditor(SCI_MARKERSETFORE, SC_MARKNUM_FOLDEROPEN, RGB(0, 0, 0));
		SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDEROPEN, RGB(0, 0, 0));
		SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_ARROW);
		SendEditor(SCI_MARKERSETFORE, SC_MARKNUM_FOLDER, RGB(0, 0, 0));
		SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDER, RGB(0, 0, 0));
	}
/*	if (extender) {
		extender->Clear();

		char defaultDir[MAX_PATH];
		GetDefaultDirectory(defaultDir, sizeof(defaultDir));
		char scriptPath[MAX_PATH];
		if (Exists(defaultDir, "SciTEGlobal.lua", scriptPath)) {
			// Found fglobal file in global directory
			extender->Load(scriptPath);
		}

		// Check for an extension script
		CStringA extensionFile = props.Get("extension.", fileNameForExtension);
		if (extensionFile.GetLength()) {
			// find file in local directory
			char docDir[MAX_PATH];
			GetDocumentDirectory(docDir, sizeof(docDir));
			if (Exists(docDir, extensionFile, scriptPath)) {
				// Found file in document directory
				extender->Load(scriptPath);
			} else if (Exists(defaultDir, extensionFile, scriptPath)) {
				// Found file in global directory
				extender->Load(scriptPath);
			} else if (Exists("", extensionFile, scriptPath)) {
				// Found as completely specified file name
				extender->Load(scriptPath);
			}
		}
	}
*/
//	firstPropertiesRead = false;
	//DWORD dwEnd = timeGetTime();
	//Platform::DebugPrintf("Properties read took %d\n", dwEnd - dwStart);

	return true;
}

// Properties that are interactively modifiable are only read from the properties file once.




void ScintillaEditor::SetStyleFor(const char *lang)
{
	for (int style = 0; style <= STYLE_MAX; style++)
	{
		if (style != STYLE_DEFAULT)
		{
			char key[200];
			sprintf(key, "style.%s.%0d", lang, style);
			CStringA sval = m_props->Get(key);
			SetOneStyle(style, sval);
		}
	}
}



bool ScintillaEditor::OpenFile(const CString& fileName)
{
	OpenFile(fileName, "");

	return true;
}


bool ScintillaEditor::OpenFile(const CString& fileName, const CString& forcedExt)
{
	ReadProperties(fileName, forcedExt);

	SendEditor(SCI_CLEARALL);

	CFile file;
	if (!file.Open(fileName, CFile::modeRead))
		return false;

	DWORD size = (DWORD)file.GetLength();
	char* buffer = new char[size];
	file.Read(buffer, size);
	file.Close();

	SendEditor(SCI_ADDTEXT, size, (LPARAM)buffer);

	delete [] buffer;

	return true;
}




void ScintillaEditor::GetRange(int start, int end, char* text)
{
	TextRange tr;
	tr.chrg.cpMin = start;
	tr.chrg.cpMax = end;
	tr.lpstrText = text;
	SendEditor(SCI_GETTEXTRANGE, 0, (LPARAM)&tr);
}


DWORD ScintillaEditor::GetSelText(char* text)
{
	return SendEditor(SCI_GETSELTEXT, 0, (LPARAM)text);
}


bool ScintillaEditor::SaveFile(const CString& fileName)
{
	CFile file;
	if (!file.Open(fileName, CFile::modeWrite | CFile::modeCreate))
		return false;

	int textSize = SendEditor(SCI_GETLENGTH);
	char* buffer = new char[textSize];

	GetRange(0, textSize - 1, buffer);

	file.Write(buffer, textSize);
	file.Close();

	delete [] buffer;

	return true;
}


CharacterRange ScintillaEditor::GetSelection()
{
	CharacterRange crange;
	crange.cpMin = SendEditor(SCI_GETSELECTIONSTART);
	crange.cpMax = SendEditor(SCI_GETSELECTIONEND);
	return crange;
}



int ScintillaEditor::GetDocumentLength()
{
	return SendEditor(SCI_GETLENGTH);
}



void ScintillaEditor::EnsureRangeVisible(int posStart, int posEnd)
{
	int lineStart = SendEditor(SCI_LINEFROMPOSITION, min(posStart, posEnd));
	int lineEnd = SendEditor(SCI_LINEFROMPOSITION, max(posStart, posEnd));
	for (int line = lineStart; line <= lineEnd; line++)
	{
		SendEditor(SCI_ENSUREVISIBLE, line);
	}
}


void ScintillaEditor::CenterPosition(int position)
{
	int numLines = SendEditor(SCI_LINESONSCREEN);
	int line = SendEditor(SCI_LINEFROMPOSITION, position);
	line -= numLines / 2;
	int lastLine = line + numLines - 1;
	for (; line <= lastLine; line++)
	{
		SendEditor(SCI_ENSUREVISIBLE, line);
	}
}


void ScintillaEditor::GotoLine(int line, bool ensureVisible)
{
	if (ensureVisible)
		SendEditor(SCI_ENSUREVISIBLE, line);
	SendEditor(SCI_GOTOLINE, line);
}


CString ScintillaEditor::GetWord()
{
	int savePos = GetCurrentPos();
	SendEditor(SCI_WORDLEFT);
	DWORD startPos = GetCurrentPos();
	SendEditor(SCI_WORDRIGHTEXTEND);
	DWORD endPos = GetCurrentPos();
	CStringA str;
	LPSTR buf = str.GetBufferSetLength(endPos - startPos);
	GetSelText(buf);
	str.ReleaseBuffer(endPos - startPos);
	SendEditor(SCI_SETCURRENTPOS, savePos);

	return CString(str);
}
