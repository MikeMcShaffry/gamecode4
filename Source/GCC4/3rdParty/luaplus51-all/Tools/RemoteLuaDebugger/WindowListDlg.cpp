// WindowListDlg.cpp : implementation file
//

#include "RemoteLuaDebugger_InternalPch.h"
#include "remoteluadebugger.h"
#include "WindowListDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWindowListDlg dialog


CWindowListDlg::CWindowListDlg(CWnd* pParent /*=NULL*/)
	: cdxCDynamicBarDlg(CWindowListDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWindowListDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CWindowListDlg::DoDataExchange(CDataExchange* pDX)
{
	cdxCDynamicBarDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWindowListDlg)
	DDX_Control(pDX, IDC_WL_LIST, m_list);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWindowListDlg, cdxCDynamicBarDlg)
	//{{AFX_MSG_MAP(CWindowListDlg)
	ON_NOTIFY(NM_DBLCLK, IDC_WL_LIST, OnDblclkWlList)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWindowListDlg message handlers


BOOL CWindowListDlg::OnInitDialog() 
{
	cdxCDynamicBarDlg::OnInitDialog();
	
	AddSzControl(m_list, mdResize, mdResize);
	m_list.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 300);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CWindowListDlg::Refresh()
{
	m_list.DeleteAllItems();

	CStringArray fileList;
	theApp.GetDocument()->GetActiveFiles(fileList);

	int fileCount = fileList.GetSize();
	for (int i = 0; i < fileCount; ++i)
	{
		m_list.InsertItem(m_list.GetItemCount(), fileList[i]);
	}
}

void CWindowListDlg::OnDblclkWlList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMLISTVIEW* info = (NMLISTVIEW*)pNMHDR;
	
	if (info->iItem != -1)
	{
		CString fileName = m_list.GetItemText(info->iItem, 0);
		theApp.GetDebuggerView()->OpenFile(fileName, 1);
	}
	
	*pResult = 0;
}

void CWindowListDlg::OnSize(UINT nType, int cx, int cy) 
{
	cdxCDynamicBarDlg::OnSize(nType, cx, cy);
	
	if (m_list.GetSafeHwnd())
	{
		CRect clientRect;
		GetClientRect(clientRect);
		m_list.SetColumnWidth(0, clientRect.Width() - 6);	
	}
}
