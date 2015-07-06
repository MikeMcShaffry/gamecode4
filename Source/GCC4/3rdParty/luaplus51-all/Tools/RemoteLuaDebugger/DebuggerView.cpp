// DebuggerView.cpp : implementation file
//

#include "RemoteLuaDebugger_InternalPch.h"
#include "RemoteLuaDebugger.h"
#include "DebuggerView.h"
#include "GotoLineDlg.h"
#include "FindDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDebuggerView

IMPLEMENT_DYNCREATE(CDebuggerView, cdxCDynamicFormView)

CDebuggerView::CDebuggerView()
	: cdxCDynamicFormView(CDebuggerView::IDD)
{
	//{{AFX_DATA_INIT(CDebuggerView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CDebuggerView::~CDebuggerView()
{
}

void CDebuggerView::DoDataExchange(CDataExchange* pDX)
{
	cdxCDynamicFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDebuggerView)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDebuggerView, cdxCDynamicFormView)
	//{{AFX_MSG_MAP(CDebuggerView)
	ON_BN_CLICKED(IDC_DB_STEPOVER, OnDbStepover)
	ON_BN_CLICKED(IDC_DB_TOGGLEBREAKPOINT, OnDbTogglebreakpoint)
	ON_BN_CLICKED(IDC_DB_STEPINTO, OnDbStepinto)
	ON_BN_CLICKED(IDC_DB_BREAK, OnDbBreak)
	ON_BN_CLICKED(IDC_DB_GO, OnDbGo)
	ON_BN_CLICKED(IDC_DB_STEPOUT, OnDbStepout)
	ON_BN_CLICKED(IDC_DB_GOTOLINE, OnDbGotoline)
	ON_BN_CLICKED(IDC_DB_FIND, OnDbFind)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(IDC_DB_FINDNEXT, OnDbFindnext)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDebuggerView diagnostics

#ifdef _DEBUG
void CDebuggerView::AssertValid() const
{
	cdxCDynamicFormView::AssertValid();
}

void CDebuggerView::Dump(CDumpContext& dc) const
{
	cdxCDynamicFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDebuggerView message handlers

void CDebuggerView::OnInitialUpdate() 
{
	cdxCDynamicFormView::OnInitialUpdate();
	
	static bool firstTime = true;

	if (firstTime)
	{
		theApp.GetMainFrame()->Init();
		CWnd* tempWnd = GetDlgItem(IDC_DB_EDITOR);
		CRect editorRect;
		tempWnd->GetWindowRect(editorRect);
		
		CRect dialogRect;
		GetClientRect(dialogRect);
		ClientToScreen(dialogRect);
		
		editorRect.OffsetRect(-dialogRect.TopLeft());
		
		m_editor.Create(GetSafeHwnd(), editorRect);

//jj		m_localsGrid.CreateGrid(this, IDC_DB_LOCALS);
//jj		m_localsGrid.Init();
//jj		m_watchGrid.CreateGrid(this, IDC_DB_WATCH);
//jj		m_watchGrid.Init();
//jj		m_watchGrid.BuildWatch("Locals");

///////////		theApp.GetMainFrame()->GetWatchBar().GetGrid().BuildWatch("Locals");

//jj		AddSzControl(m_localsGrid, 100, 0, 100, 50);
//jj		AddSzControl(m_watchGrid, 100, 50, 100, 100);
//jj		AddSzControl(IDC_DB_LOCALSSTATIC, 100, 0, 100, 0);
//jj		AddSzControl(IDC_DB_WATCHSTATIC, 100, 50, 100, 50);
//		AddSzControl(m_actionList, 0, 0, 0, 50);
//		AddSzControl(m_generatedActionList, 0, 50, 0, 100);
		AddSzControl(m_editor.GetWindowHandle(), mdResize, mdResize);
//		AddSzControl(m_actionGrid->GetSafeHwnd(), mdResize, mdResize);

//		m_actionList.SetExtendedStyle(m_actionList.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
//		m_actionList.InsertColumn(0, "Actions", LVCFMT_LEFT, 200);
//		m_generatedActionList.SetExtendedStyle(m_generatedActionList.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
//		m_generatedActionList.InsertColumn(0, "Generated Actions", LVCFMT_LEFT, 200);

		firstTime = false;

//		m_gridActive = true;
//		::ShowWindow(m_editor.GetWindowHandle(), SW_HIDE);

		tempWnd->DestroyWindow();

		m_editor.SendEditor(SCI_SETFOCUS, 1);
	}
}

int m_lastLineNumber = -1;
#define MARKER_BREAKPOINT 1
#define MARKER_CURRENTLINE 2


void CDebuggerView::OpenFile(const CString& fileName, int lineNumber)
{
	// Turn off the marker for the last line.
	m_editor.SendEditor(SCI_MARKERDELETE, m_lastLineNumber, MARKER_CURRENTLINE);

	CString shortFileName = fileName;
	int slashPos = shortFileName.ReverseFind('\\');
	if (slashPos != -1)
		shortFileName = shortFileName.Mid(slashPos + 1);
	slashPos = shortFileName.ReverseFind('/');
	if (slashPos != -1)
		shortFileName = shortFileName.Mid(slashPos + 1);
	
	if (m_currentFileName != fileName)
	{
		m_editor.SendEditor(SCI_SETREADONLY, 0);
		m_editor.OpenFile(theApp.GetCachePath() + shortFileName, "lua");
		m_editor.SendEditor(SCI_SETMARGINWIDTHN, 0, 0);
		m_editor.SendEditor(SCI_SETMARGINWIDTHN, 1, 20);
		m_editor.SendEditor(SCI_SETREADONLY, 1);
		m_currentFileName = fileName;

		DrawAllBreakpoints();
	}

	// no need to do each time, but can't hurt either :-)
	m_editor.SendEditor(SCI_MARKERDEFINE, MARKER_CURRENTLINE, SC_MARK_SHORTARROW);
	m_editor.SendEditor(SCI_MARKERSETFORE, MARKER_CURRENTLINE, RGB(0x00, 0x00, 0));
	m_editor.SendEditor(SCI_MARKERSETBACK, MARKER_CURRENTLINE, RGB(0xff, 0xff, 0x00));
	m_editor.SendEditor(SCI_MARKERADD, lineNumber, MARKER_CURRENTLINE);
	m_lastLineNumber = lineNumber;

	m_editor.GotoLine(lineNumber, true);

	m_ft.chrg.cpMin = 0;
	m_ft.chrg.cpMax = m_editor.GetDocumentLength();
	m_ft.chrgText.cpMin = 0;
	m_ft.chrgText.cpMax = 0;
}


void CDebuggerView::DebugSetLocation(const CStringW& stateToDebug, const CString& fileName, int lineNumber)
{
	m_stateToDebug = stateToDebug;
	OpenFile(fileName, lineNumber);
	theApp.GetMainFrame()->SetActiveWindow();
	theApp.GetMainFrame()->SetForegroundWindow();
	this->SetFocus();
}


int CDebuggerView::LS_DebugUpdateLocals( LuaState* state )
{
	LuaStackObject tableObj( state, 2 );
	theApp.GetMainFrame()->GetLocalsBar().GetGrid().BuildLocals( tableObj );

	return 0;
}


int CDebuggerView::LS_DebugUpdateWatch( LuaState* state )
{
	LuaStack args(state);
	CString stateName = args[1].GetString();

	//??
	theApp.GetMainFrame()->GetWatchBar().GetGrid().UpdateWatch( args[2] );

	return 0;
}


void CDebuggerView::DebugUpdateCallStack(const CStringW& stateNameToDebug)
{
	BYTE* buffer;
	int size;
	theApp.GetNetworkClient().GetSocket().ReadBinary(buffer, size);      //jj = atoi(status);
	
	if (size > 0)
	{
		theApp.GetDocument()->GetState()->DoString((char*)buffer);
		theApp.GetMainFrame()->GetCallStackBar().Update();
		
		delete [] buffer;
	}
}


void CDebuggerView::DebugAddAllWatches(const CStringW& stateNameToDebug)
{
	m_watchGrid.DebugAddAllWatches(stateNameToDebug);
}


struct BreakpointInfo
{
	CString m_fileName;
	int m_lineNumber;
};

CArray<BreakpointInfo, BreakpointInfo&> m_breakpointInfo;


int FindBreakpoint(const CString& fileName, int lineNumber)
{
	int count = m_breakpointInfo.GetSize();
	for (int i = 0; i < count; ++i)
	{
		BreakpointInfo& info = m_breakpointInfo[i];
		if (info.m_fileName == fileName  &&  info.m_lineNumber == lineNumber)
			return i;
	}
	
	return -1;
}


void CDebuggerView::DebugSendAllBreakpoints(const CStringW& stateNameToDebug)
{
	int count = m_breakpointInfo.GetSize();
	for (int i = 0; i < count; ++i)
	{
		BreakpointInfo& info = m_breakpointInfo[i];
		CString command;
		command.Format(_T("DebugSetBreakpoint('%s',%d,true)"), info.m_fileName, info.m_lineNumber);
		theApp.GetNetworkClient().SendCommand(command);
	}

	m_watchGrid.DebugAddAllWatches(stateNameToDebug);
}


void CDebuggerView::DrawAllBreakpoints() 
{
	int count = m_breakpointInfo.GetSize();
	for (int i = 0; i < count; ++i)
	{
		BreakpointInfo& info = m_breakpointInfo[i];
		if (info.m_fileName == m_currentFileName)
		{
			m_editor.SendEditor(SCI_MARKERDEFINE, MARKER_BREAKPOINT, SC_MARK_CIRCLE);
			m_editor.SendEditor(SCI_MARKERSETFORE, MARKER_BREAKPOINT, RGB(0x00, 0x00, 0));
			m_editor.SendEditor(SCI_MARKERSETBACK, MARKER_BREAKPOINT, RGB(0xff, 0x00, 0x00));
			m_editor.SendEditor(SCI_MARKERADD, info.m_lineNumber, MARKER_BREAKPOINT);
		}
	}
}



void CDebuggerView::OnDbTogglebreakpoint() 
{
	int pos = m_editor.SendEditor(SCI_GETCURRENTPOS);
	int lineNumber = m_editor.SendEditor(SCI_LINEFROMPOSITION, pos);

	// Is there a breakpoint currently here?
	int breakpointIndex = FindBreakpoint(m_currentFileName, lineNumber);
	if (breakpointIndex == -1)
	{
		// No, add it.
		m_editor.SendEditor(SCI_MARKERDEFINE, MARKER_BREAKPOINT, SC_MARK_CIRCLE);
		m_editor.SendEditor(SCI_MARKERSETFORE, MARKER_BREAKPOINT, RGB(0x00, 0x00, 0));
		m_editor.SendEditor(SCI_MARKERSETBACK, MARKER_BREAKPOINT, RGB(0xff, 0x00, 0x00));
		m_editor.SendEditor(SCI_MARKERADD, lineNumber, MARKER_BREAKPOINT);

		// Add the breakpoint.
		BreakpointInfo info;
		info.m_fileName = m_currentFileName;
		info.m_lineNumber = lineNumber;
		m_breakpointInfo.Add(info);
	}
	else
	{
		// Remove the breakpoint.
		m_editor.SendEditor(SCI_MARKERDELETE, lineNumber, MARKER_BREAKPOINT);
		m_breakpointInfo.RemoveAt(breakpointIndex);
	}

	CString command;
	command.Format(_T("DebugSetBreakpoint('%s',%d,%s)"), m_currentFileName, lineNumber,
		(breakpointIndex == -1) ? _T("true") : _T("false"));
	theApp.GetNetworkClient().SendCommand(command);
}


void CDebuggerView::OnDbStepover() 
{
	CString command;
	command.Format(_T("DebugStepOver()"));
	theApp.GetNetworkClient().SendCommand(command);
}

void CDebuggerView::OnDbStepinto() 
{
	CString command;
	command.Format(_T("DebugStepInto()"));
	theApp.GetNetworkClient().SendCommand(command);
}

void CDebuggerView::OnDbBreak() 
{
	CString command;
	command.Format(_T("DebugBreak()"));
	theApp.GetNetworkClient().SendCommand(command);
}

void CDebuggerView::OnDbGo() 
{
	CString command;
	command.Format(_T("DebugGo()"));
	theApp.GetNetworkClient().SendCommand(command);

	// Turn off the marker for the last line.
	m_editor.SendEditor(SCI_MARKERDELETE, m_lastLineNumber, MARKER_CURRENTLINE);
}

void CDebuggerView::OnDbStepout() 
{
	CString command;
	command.Format(_T("DebugStepOut()"));
	theApp.GetNetworkClient().SendCommand(command);

	// Turn off the marker for the last line.
	m_editor.SendEditor(SCI_MARKERDELETE, m_lastLineNumber, MARKER_CURRENTLINE);
}

void CDebuggerView::OnDbGotoline() 
{
	CGotoLineDlg dlg;
	dlg.DoModal();
	::SetFocus(m_editor.GetWindowHandle());
}

void CDebuggerView::OnDbFind() 
{
	CFindDlg dlg;
	dlg.DoModal();
	::SetFocus(m_editor.GetWindowHandle());
}

void CDebuggerView::OnEditCopy() 
{
	m_editor.SendEditor(SCI_COPY);
}

void CDebuggerView::OnDbFindnext() 
{
	FindNext();
}

void CDebuggerView::FindNext()
{
	if (m_findText.IsEmpty())
		return;

	m_ft.chrg.cpMin = m_ft.chrgText.cpMax;
	m_ft.chrg.cpMax = m_editor.GetDocumentLength();
	m_ft.chrgText.cpMin = 0;
	m_ft.chrgText.cpMax = m_editor.GetCurrentPos();
	m_ft.lpstrText = (LPSTR)(LPCTSTR)m_findText;

	int posFind = m_editor.SendEditor(SCI_FINDTEXT, 0, reinterpret_cast<long>(&m_ft));
	if (posFind != -1)
	{
		m_editor.CenterPosition(m_ft.chrgText.cpMin);
		m_editor.SetSelection(m_ft.chrgText.cpMin, m_ft.chrgText.cpMax);
	}
	else
	{
		m_ft.chrg.cpMin = 0;
		m_ft.chrg.cpMax = m_editor.GetDocumentLength();
		m_ft.chrgText.cpMin = 0;
		m_ft.chrgText.cpMax = 0;
	}
}