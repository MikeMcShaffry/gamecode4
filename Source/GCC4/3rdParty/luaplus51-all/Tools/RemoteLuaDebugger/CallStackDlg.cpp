// CallStackDlg.cpp : implementation file
//

#include "RemoteLuaDebugger_InternalPch.h"
#include "remoteluadebugger.h"
#include "CallStackDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCallStackDlg dialog


CCallStackDlg::CCallStackDlg(CWnd* pParent /*=NULL*/)
	: cdxCDynamicBarDlg(CCallStackDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCallStackDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCallStackDlg::DoDataExchange(CDataExchange* pDX)
{
	cdxCDynamicBarDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCallStackDlg)
	DDX_Control(pDX, IDC_CS_LIST, m_list);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCallStackDlg, cdxCDynamicBarDlg)
	//{{AFX_MSG_MAP(CCallStackDlg)
	ON_WM_SIZE()
	ON_NOTIFY(NM_DBLCLK, IDC_CS_LIST, OnDblclkCsList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCallStackDlg message handlers

BOOL CCallStackDlg::OnInitDialog() 
{
	cdxCDynamicBarDlg::OnInitDialog();
	
	AddSzControl(m_list, mdResize, mdResize);
	m_list.InsertColumn(0, _T("Function"), LVCFMT_LEFT, 300);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCallStackDlg::OnSize(UINT nType, int cx, int cy) 
{
	cdxCDynamicBarDlg::OnSize(nType, cx, cy);
	
	if (m_list.GetSafeHwnd())
	{
		CRect listWindowRect;
		CRect listClientRect;
		m_list.GetWindowRect(listWindowRect);
		m_list.GetClientRect(listClientRect);
		m_list.ClientToScreen(listClientRect);
		m_list.SetColumnWidth(0, listWindowRect.Width() - (listWindowRect.Width() - listClientRect.Width()));
	}
}

void CCallStackDlg::OnDblclkCsList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}


void CCallStackDlg::Update()
{
	m_list.DeleteAllItems();

	LuaState* state = theApp.GetDocument()->GetState();

	LuaObject callStackTableObj = state->GetGlobals()[ "CallStack" ];
	if (callStackTableObj.IsTable())
	{
		int index = 1;
		while (true)
		{
			LuaObject callTableObj = callStackTableObj.GetByIndex(index++);
			if (callTableObj.IsNil())
				break;
			LuaObject nameObj = callTableObj.GetByIndex(1);
			LuaObject typeObj = callTableObj.GetByIndex(2);
			LuaObject lineNumberObj = callTableObj.GetByIndex(3);

			CString str;
			if (lineNumberObj.GetInteger() != -1)
				str.Format(_T(" line %d"), lineNumberObj.GetInteger());
			str = nameObj.GetString() + str + _T(" (") + typeObj.GetString() + _T(")");
			m_list.InsertItem(m_list.GetItemCount(), str);
		}
	}
}
