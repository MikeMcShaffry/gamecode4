// GotoLineDlg.cpp : implementation file
//

#include "RemoteLuaDebugger_InternalPch.h"
#include "remoteluadebugger.h"
#include "GotoLineDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGotoLineDlg dialog


CGotoLineDlg::CGotoLineDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGotoLineDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGotoLineDlg)
	m_lineNumberEdit = 0;
	//}}AFX_DATA_INIT
}


void CGotoLineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGotoLineDlg)
	DDX_Text(pDX, IDC_GL_LINENUMBER, m_lineNumberEdit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGotoLineDlg, CDialog)
	//{{AFX_MSG_MAP(CGotoLineDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGotoLineDlg message handlers

void CGotoLineDlg::OnOK() 
{
	UpdateData(TRUE);

	ScintillaEditor& editor = theApp.GetDocument()->GetDebuggerView()->GetEditor();
	int lineNumber = m_lineNumberEdit - 1;
	editor.GotoLine(lineNumber, true);
	editor.SetSelection(editor.GetLineEndPosition(lineNumber), editor.PositionFromLine(lineNumber));
	editor.CenterPosition(editor.PositionFromLine(lineNumber));
		
	CDialog::OnOK();
}
