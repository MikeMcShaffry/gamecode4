// cdxCDynamicBar.cpp : implementation file
//

#include "stdafx.h"
#include "cdxCDynamicBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicBarDlg dialog
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(cdxCDynamicBarDlg,cdxCDynamicChildDlg);

/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicBarDlg dialog
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(cdxCDynamicBarDlg, cdxCDynamicChildDlg)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicBarDlg functions
/////////////////////////////////////////////////////////////////////////////

bool cdxCDynamicBarDlg::Create(cdxCDynamicBar *pBar)
{
	return cdxCDynamicChildDlg::Create(m_nID,(CWnd *)pBar) != FALSE;
}





/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicBar
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(cdxCDynamicBar,basecdxCDynamicBar);

/////////////////////////////////////////////////////////////////////////////
// construction
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(cdxCDynamicBar, basecdxCDynamicBar)
	//{{AFX_MSG_MAP(cdxCDynamicBar)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicBar message handlers
/////////////////////////////////////////////////////////////////////////////

int cdxCDynamicBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (basecdxCDynamicBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	// set single child autosizing mode
	SetSCBStyle(GetSCBStyle() | SCBS_SIZECHILD);

	// create the child dialog
	if (!m_rDlg.Create(this))
		return -1;

	ASSERT(::IsWindow(m_hWnd));
	ASSERT(::IsWindow(m_rDlg.m_hWnd));

	// allow image caching when resizing controls (reduces flicker)
	m_rDlg.ModifyFlags(cdxCDynamicWnd::flSWPCopyBits, 0);

	// set the initial&min sizes for the bar, based on child dialog size
	//   NcCalcClient() is called to compute the extra non-client space
	//   needed - r.Size() will be negative after each call
	CRect r;
	r.SetRectEmpty();
	NcCalcClient(&r, AFX_IDW_DOCKBAR_FLOAT);
    m_szMinFloat = m_szFloat = m_rDlg.m_szMin - r.Size();

	r.SetRectEmpty();
	NcCalcClient(&r, AFX_IDW_DOCKBAR_LEFT);
    m_szMinVert = m_szVert = m_rDlg.m_szMin - r.Size();

	r.SetRectEmpty();
	NcCalcClient(&r, AFX_IDW_DOCKBAR_TOP);
	m_szMinHorz = m_szHorz = m_rDlg.m_szMin - r.Size();

	return 0;
}
		  
/*
 * route command UI updates to dialog
 */

void cdxCDynamicBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	basecdxCDynamicBar::OnUpdateCmdUI(pTarget,bDisableIfNoHndler);
	if(m_rDlg.IsWindow())
		m_rDlg.OnUpdateCmdUI(pTarget,bDisableIfNoHndler);
}

/*
 * route commands a long...
 */

BOOL cdxCDynamicBar::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	if(m_rDlg.IsWindow() && m_rDlg.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;
	return basecdxCDynamicBar::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

/*
 * route commands ...
 */

BOOL cdxCDynamicBar::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if(m_rDlg.IsWindow() && m_rDlg.OnCommand(wParam, lParam))
		return TRUE;
	return basecdxCDynamicBar::OnCommand(wParam, lParam);
}
