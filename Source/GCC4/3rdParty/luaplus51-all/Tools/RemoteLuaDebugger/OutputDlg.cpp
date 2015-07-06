// OutputDlg.cpp : implementation file
//

#include "RemoteLuaDebugger_InternalPch.h"
#include "RemoteLuaDebugger.h"
#include "OutputDlg.h"


// COutputDlg dialog

COutputDlg::COutputDlg(CWnd* pParent /*=NULL*/)
	: cdxCDynamicBarDlg(COutputDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COutputDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

COutputDlg::~COutputDlg()
{
}

void COutputDlg::DoDataExchange(CDataExchange* pDX)
{
	cdxCDynamicBarDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COutputDlg)
	DDX_Control(pDX, IDC_EDIT_OUTPUT, m_outputEdit);
	//}}AFX_DATA_MAP
}


/////////////////////////////////////////////////////////////////////////////
// CCallStackDlg message handlers

BEGIN_MESSAGE_MAP(COutputDlg, cdxCDynamicBarDlg)
	//{{AFX_MSG_MAP(CCallStackDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL COutputDlg::OnInitDialog() 
{
	cdxCDynamicBarDlg::OnInitDialog();
	
	AddSzControl(m_outputEdit, mdResize, mdResize);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void COutputDlg::Append( const CString& str )
{
	// Append string
	CString   strBuffer;    // current contents of edit control
	m_outputEdit.GetWindowText( strBuffer );
	strBuffer += str;
	m_outputEdit.SetWindowText( strBuffer );

	// Scroll the edit control
	m_outputEdit.LineScroll( m_outputEdit.GetLineCount(), 0 );
}	

void COutputDlg::AppendLuaCode( const CString& str )
{
	// Append string
	CString   strBuffer;    // current contents of edit control
	m_outputEdit.GetWindowText( strBuffer );

	// We need to replace \n's with \r\n's
	CString strClean = str;
	strClean.Replace( _T("\n"), _T("\r\n") );

	strBuffer += strClean;
	m_outputEdit.SetWindowText( strBuffer );

	// Scroll the edit control
	m_outputEdit.LineScroll( m_outputEdit.GetLineCount(), 0 );
}


// COutputDlg message handlers
