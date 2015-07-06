// CommandLineDlg.cpp : implementation file
//

#include "RemoteLuaDebugger_InternalPch.h"
#include "RemoteLuaDebugger.h"
#include "CommandLineDlg.h"

//
// CCommandLineInputEdit edit control
//
BEGIN_MESSAGE_MAP(CCommandLineInputEdit, CACEditCtrl)
	//{{AFX_MSG_MAP(CCommandLineInputEdit)
	ON_WM_GETDLGCODE()
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static CString getLuaCleanString( const CString& strIn )
{
	CString strOut;
	strOut.AppendChar( '"' );
	for ( int i = 0; i < strIn.GetLength(); i++ )
	{
		TCHAR ch = strIn[i];
		switch ( ch )
		{
		case '\"':	case '\\':	case '\'':	case '[':	case ']':
			strOut.AppendChar( '\\' );
			strOut.AppendChar( ch );
			break;
		case '\n':	strOut.Append( _T("\\n") );	break;
		case '\a':	strOut.Append( _T("\\a") );	break;
		case '\f':	strOut.Append( _T("\\f") );	break;
		case '\r':	strOut.Append( _T("\\r") );	break;
		case '\t':	strOut.Append( _T("\\t") );	break;
		case '\v':	strOut.Append( _T("\\v") );	break;
		default:
			if ( isprint(ch) )
				strOut.AppendChar( ch );
			else
			{
				CString strTemp;
				strTemp.Format(_T("\\%03d"), ch );
				strOut += strTemp;
			}
		}
	}
	strOut += "\"";
	return strOut;
}

UINT CCommandLineInputEdit::OnGetDlgCode()
{
	return DLGC_WANTALLKEYS;
}


// Execute the buffer on control-enter
void CCommandLineInputEdit::OnChar( UINT nChar, UINT nRepCnt, UINT nFlags ) 
{
	// Check for control-enter, which for some reason is vk-code 0x0A (emperical)
	if ( nChar == 0x0A )
	{
		// Get the edit box text
		CString editString;
		this->GetWindowText( editString );

		// Only execute selection if there is one
		int nStartChar, nEndChar;
		this->GetSel( nStartChar, nEndChar );
		if ( nStartChar != nEndChar )
			editString = editString.Mid( nStartChar, nEndChar-nStartChar );

		// Execute a Lua-clean command buffer on the client
//		CString cleanString = getLuaCleanString( editString );
		CString command;
		command.Format(_T("DebugDoBuffer('%s',[=====[%s]=====])"), theApp.GetDebuggerView()->GetStateToDebug(), editString );
		theApp.GetNetworkClient().SendCommand( command );

		AddACEntry( editString );
		return;
	}
	CACEditCtrl::OnChar( nChar, nRepCnt, nFlags );
}



// CCommandLineDlg dialog
CCommandLineDlg::CCommandLineDlg(CWnd* pParent /*=NULL*/)
	: cdxCDynamicBarDlg(CCommandLineDlg::IDD, pParent)
{
}

CCommandLineDlg::~CCommandLineDlg()
{
}

void CCommandLineDlg::DoDataExchange(CDataExchange* pDX)
{
	cdxCDynamicBarDlg::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_INPUT, m_editCommand);
}

BOOL CCommandLineDlg::OnInitDialog() 
{
	cdxCDynamicBarDlg::OnInitDialog();
	AddSzControl(m_editCommand, mdResize, mdResize);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


BEGIN_MESSAGE_MAP(CCommandLineDlg, cdxCDynamicBarDlg)
END_MESSAGE_MAP()


// CCommandLineDlg message handlers
