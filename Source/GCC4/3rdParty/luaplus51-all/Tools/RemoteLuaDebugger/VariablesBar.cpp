// VariablesBar.cpp : implementation file
//

#include "RemoteLuaDebugger_InternalPch.h"
#include "RemoteLuaDebugger.h"
#include "VariablesBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVariablesBar dialog


CVariablesBar::CVariablesBar(CWnd* pParent /*=NULL*/)
	: cdxCDynamicBarDlg(CVariablesBar::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVariablesBar)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CVariablesBar::DoDataExchange(CDataExchange* pDX)
{
	cdxCDynamicBarDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVariablesBar)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVariablesBar, cdxCDynamicBarDlg)
	//{{AFX_MSG_MAP(CVariablesBar)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVariablesBar message handlers

BOOL CVariablesBar::OnInitDialog() 
{
	cdxCDynamicBarDlg::OnInitDialog();
	
	m_grid.CreateGrid(this, IDC_VB_GRID);
	AddSzControl(m_grid, mdResize, mdResize);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CVariablesBar::Init()
{
	m_grid.Init();
}