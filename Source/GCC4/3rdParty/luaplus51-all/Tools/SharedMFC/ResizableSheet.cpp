// ResizableSheet.cpp : implementation file
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2000 by Paolo Messina
// (ppescher@yahoo.com)
//
// Free for non-commercial use.
// You may change the code to your needs,
// provided that credits to the original 
// author is given in the modified files.
//  
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ResizableSheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResizableSheet

IMPLEMENT_DYNAMIC(CResizableSheet, CPropertySheet)

inline void CResizableSheet::Construct()
{
	m_bInitDone = FALSE;

	m_bUseMinTrack = TRUE;
	m_bUseMaxTrack = FALSE;
	m_bUseMaxRect = FALSE;

	m_bShowGrip = TRUE;
	
	m_bEnableSaveRestore = FALSE;
	m_bSavePage = FALSE;

	m_szGripSize.cx = GetSystemMetrics(SM_CXVSCROLL);
	m_szGripSize.cy = GetSystemMetrics(SM_CYHSCROLL);
}


CResizableSheet::CResizableSheet()
{
	Construct();
}

CResizableSheet::CResizableSheet(UINT nIDCaption, CWnd *pParentWnd, UINT iSelectPage)
	 : CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	Construct();
}

CResizableSheet::CResizableSheet(LPCTSTR pszCaption, CWnd *pParentWnd, UINT iSelectPage)
	 : CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	Construct();
}

CResizableSheet::~CResizableSheet()
{
}

BEGIN_MESSAGE_MAP(CResizableSheet, CPropertySheet)
	//{{AFX_MSG_MAP(CResizableSheet)
	ON_WM_PAINT()
	ON_WM_NCHITTEST()
	ON_WM_GETMINMAXINFO()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(ID_WIZBACK, OnPageChanged)
	ON_BN_CLICKED(ID_WIZNEXT, OnPageChanged)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResizableSheet message handlers

int CResizableSheet::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CPropertySheet::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// change window style to be resizable
	ModifyStyle(0, WS_THICKFRAME | WS_CLIPCHILDREN);

	return 0;
}

BOOL CResizableSheet::OnInitDialog() 
{
	BOOL bResult = CPropertySheet::OnInitDialog();
	
	CRect rc;
	GetWindowRect(&rc);

	// set the initial size as the min track size
	m_ptMinTrackSize.x = rc.Width();
	m_ptMinTrackSize.y = rc.Height();

	UpdateGripPos();

	PresetLayout();

	// prevent flickering
	GetTabControl()->ModifyStyle(0, WS_CLIPSIBLINGS);

	m_bInitDone = TRUE;

	return bResult;
}

void CResizableSheet::OnDestroy() 
{
	CPropertySheet::OnDestroy();

	if (m_bEnableSaveRestore)
		SaveWindowRect();
}

// maps an index to a button ID and vice-versa
static UINT _propButtons[] =
{
	IDOK, IDCANCEL, ID_APPLY_NOW, IDHELP,
	ID_WIZBACK, ID_WIZNEXT, ID_WIZFINISH
};

// horizontal line in wizard mode
#define ID_WIZLINE	ID_WIZFINISH+1

void CResizableSheet::PresetLayout()
{
	CWnd* pWnd;	// points to various children
	CRect wndrc, objrc;
	GetClientRect(&wndrc);

	// tab control or wizard line position
	if (m_psh.dwFlags & PSH_WIZARD)	// wizard mode
	{
		// get wizard line's bottom-right corner
		pWnd = GetDlgItem(ID_WIZLINE);

		// hide tab control
		GetTabControl()->ShowWindow(SW_HIDE);
	}
	else	// tabbed mode
	{
		// get tab control's bottom-right corner
		pWnd = GetTabControl();
	}
	// whatever it is, take the right margin
	pWnd->GetWindowRect(&objrc);
	ScreenToClient(&objrc);

	m_szLayoutTabLine.cx = objrc.right - wndrc.right;
	m_szLayoutTabLine.cy = objrc.bottom - wndrc.bottom;

	// get child dialog's bottom-right corner
	pWnd = GetActivePage();

	pWnd->GetWindowRect(&objrc);
	ScreenToClient(&objrc);

	m_szLayoutPage.cx = objrc.right - wndrc.right;
	m_szLayoutPage.cy = objrc.bottom - wndrc.bottom;

	// store buttons position
	for (int i = 0; i < 7; i++)
	{
		pWnd = GetDlgItem(_propButtons[i]);
		
		if (pWnd == NULL)
		{
			// invalid position, button does not exist
			// (just to initialize, any button you may activate
			// in the future is present, but hidden)
			m_szLayoutButton[i].cx = 0;
			m_szLayoutButton[i].cy = 0;
			continue;
		}
		
		pWnd->GetWindowRect(&objrc);
		ScreenToClient(&objrc);

		m_szLayoutButton[i].cx = objrc.left - wndrc.right;
		m_szLayoutButton[i].cy = objrc.top - wndrc.bottom;
	}
}

void CResizableSheet::ArrangeLayout()
{
	// init some vars
	CWnd* pWnd;
	CRect wndrc, objrc;
	GetClientRect(&wndrc);

	// usually no more than
	// 4 buttons +
	// 1 tab control or wizard line +
	// 1 active page
	HDWP hdwp = BeginDeferWindowPos(6);

	// assume wizard mode
	CPoint ptPagePos(0,0);	// holds a page's top-left corner
	DWORD dwPageFlags = SWP_NOMOVE;

	if (m_psh.dwFlags & PSH_WIZARD)	// wizard mode
	{
		// get wizard line's bottom-right corner
		pWnd = GetDlgItem(ID_WIZLINE);

		pWnd->GetWindowRect(&objrc);
		ScreenToClient(&objrc);

		int oldHeight = objrc.Height();
		objrc.right = m_szLayoutTabLine.cx + wndrc.right;
		objrc.bottom = m_szLayoutTabLine.cy + wndrc.bottom;
		objrc.top = objrc.bottom - oldHeight;

		// add the control
		DeferWindowPos(hdwp, *pWnd, NULL, objrc.left, objrc.top,
			objrc.Width(), objrc.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else	// tabbed mode
	{
		// get tab control's bottom-right corner
		pWnd = GetTabControl();

		pWnd->GetWindowRect(&objrc);
		ScreenToClient(&objrc);

		objrc.right = m_szLayoutTabLine.cx + wndrc.right;
		objrc.bottom = m_szLayoutTabLine.cy + wndrc.bottom;

		// add the control, only resize
		DeferWindowPos(hdwp, *pWnd, NULL, 0, 0, objrc.Width(),
			objrc.Height(), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

		EndDeferWindowPos(hdwp);
		hdwp = BeginDeferWindowPos(5);

		// correct page position (for stacked tabs)
		pWnd->GetWindowRect(&objrc);
		((CTabCtrl*)pWnd)->AdjustRect(FALSE, &objrc);
		ScreenToClient(&objrc);
		ptPagePos = objrc.TopLeft();
		dwPageFlags = 0;
	}

	// get child dialog's bottom-right corner
	pWnd = GetActivePage();

	pWnd->GetWindowRect(&objrc);
	ScreenToClient(&objrc);

	objrc.right = m_szLayoutPage.cx + wndrc.right;
	objrc.bottom = m_szLayoutPage.cy + wndrc.bottom;

	if (dwPageFlags == 0)	// correct pos
	{
		objrc.left = ptPagePos.x;
		objrc.top = ptPagePos.y;
	}

	// add the page, with proper attributes
	DeferWindowPos(hdwp, *pWnd, NULL, ptPagePos.x, ptPagePos.y, objrc.Width(),
		objrc.Height(), dwPageFlags | SWP_NOZORDER | SWP_NOACTIVATE);

	// arrange buttons position
	for (int i = 0; i < 7; i++)
	{
		pWnd = GetDlgItem(_propButtons[i]);
		
		if (pWnd == NULL)
			continue;	// ignores deleted buttons

		// this should never happen, because all the buttons you
		// may activate already exist at time PresetLayout is called
		ASSERT(m_szLayoutButton[i].cx != 0 || m_szLayoutButton[i].cy != 0);
		
		pWnd->GetWindowRect(&objrc);
		ScreenToClient(&objrc);

		objrc.left = m_szLayoutButton[i].cx + wndrc.right;
		objrc.top = m_szLayoutButton[i].cy + wndrc.bottom;

		// add the control, only move
		DeferWindowPos(hdwp, *pWnd, NULL, objrc.left, objrc.top,
			0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	// update size-grip
	InvalidateRect(&m_rcGripRect);
	UpdateGripPos();
	InvalidateRect(&m_rcGripRect);

	// go re-arrange child windows
	EndDeferWindowPos(hdwp);
}

void CResizableSheet::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
	if (nType == SIZE_MAXHIDE || nType == SIZE_MAXSHOW)
		return;		// arrangement not needed

	if (m_bInitDone)
		ArrangeLayout();
}

// only gets called in wizard mode
// (when back or next button pressed)
void CResizableSheet::OnPageChanged()
{
	// call default handler to allow page change
	Default();

	// update new wizard page
	ArrangeLayout();
}

void CResizableSheet::OnPaint() 
{
	CPaintDC dc(this);

	if (m_bShowGrip && !IsZoomed())
	{
		// draw size-grip
		dc.DrawFrameControl(&m_rcGripRect, DFC_SCROLL, DFCS_SCROLLSIZEGRIP);
	}
}

LRESULT CResizableSheet::OnNcHitTest(CPoint point) 
{
	CPoint pt = point;
	ScreenToClient(&pt);

	if (m_bShowGrip && m_rcGripRect.PtInRect(pt))
		return HTBOTTOMRIGHT;
	
	return CPropertySheet::OnNcHitTest(point);
}

void CResizableSheet::UpdateGripPos()
{
	// size-grip goes bottom right in the client area

	GetClientRect(&m_rcGripRect);

	m_rcGripRect.left = m_rcGripRect.right - m_szGripSize.cx;
	m_rcGripRect.top = m_rcGripRect.bottom - m_szGripSize.cy;
}

void CResizableSheet::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	if (!m_bInitDone)
		return;

	if (m_bUseMinTrack)
		lpMMI->ptMinTrackSize = m_ptMinTrackSize;

	if (m_bUseMaxTrack)
		lpMMI->ptMaxTrackSize = m_ptMaxTrackSize;

	if (m_bUseMaxRect)
	{
		lpMMI->ptMaxPosition = m_ptMaxPos;
		lpMMI->ptMaxSize = m_ptMaxSize;
	}
}

// protected members

void CResizableSheet::SetMaximizedRect(const CRect& rc)
{
	m_bUseMaxRect = TRUE;
	m_ptMaxPos = rc.TopLeft();
	
	CSize sz = rc.Size();
	m_ptMaxSize.x = sz.cx;
	m_ptMaxSize.y = sz.cy;
}

void CResizableSheet::ResetMaximizedRect()
{
	m_bUseMaxRect = FALSE;
}

void CResizableSheet::ShowSizeGrip(BOOL bShow)
{
	if (m_bShowGrip != bShow)
	{
		m_bShowGrip = bShow;
		InvalidateRect(&m_rcGripRect);
	}
}

void CResizableSheet::SetMinTrackSize(const CSize& size)
{
	m_bUseMinTrack = TRUE;

	m_ptMinTrackSize.x = size.cx;
	m_ptMinTrackSize.y = size.cy;
}

void CResizableSheet::ResetMinTrackSize()
{
	m_bUseMinTrack = FALSE;
}

void CResizableSheet::SetMaxTrackSize(const CSize& size)
{
	m_bUseMaxTrack = TRUE;

	m_ptMaxTrackSize.x = size.cx;
	m_ptMaxTrackSize.y = size.cy;
}

void CResizableSheet::ResetMaxTrackSize()
{
	m_bUseMaxTrack = FALSE;
}

// NOTE: this must be called after all the other settings
//       to have the dialog and its controls displayed properly
void CResizableSheet::EnableSaveRestore(LPCTSTR pszSection, LPCTSTR pszEntry, BOOL bWithPage)
{
	m_sSection = pszSection;
	m_sEntry = pszEntry;
	m_bSavePage = bWithPage;

	m_bEnableSaveRestore = TRUE;

	LoadWindowRect();
}

// private memebers

// used to save/restore window's size and position
// either in the registry or a private .INI file
// depending on your application settings

#define PROFILE_FMT 	_T("%d,%d,%d,%d,%d,%d [%d]")

void CResizableSheet::SaveWindowRect()
{
	CString data;
	WINDOWPLACEMENT wp;

	ZeroMemory(&wp, sizeof(WINDOWPLACEMENT));
	wp.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(&wp);
	
	RECT& rc = wp.rcNormalPosition;	// alias

	// also saves active page index, zero (the first) if problems
	// cannot use GetActivePage, because it always fails
	CTabCtrl *pTab = GetTabControl();
	int page = 0;

	if (pTab != NULL) 
		page = pTab->GetCurSel();
	if (page < 0)
		page = 0;

	// always save page
	data.Format(PROFILE_FMT, rc.left, rc.top,
		rc.right, rc.bottom, wp.showCmd, wp.flags, page);

	AfxGetApp()->WriteProfileString(m_sSection, m_sEntry, data);
}

void CResizableSheet::LoadWindowRect()
{
	CString data;
	WINDOWPLACEMENT wp;
	int page;

	data = AfxGetApp()->GetProfileString(m_sSection, m_sEntry);
	
	if (data.IsEmpty())	// never saved before
		return;
	
	ZeroMemory(&wp, sizeof(WINDOWPLACEMENT));
	wp.length = sizeof(WINDOWPLACEMENT);

	RECT& rc = wp.rcNormalPosition;	// alias

	if (_stscanf(data, PROFILE_FMT, &rc.left, &rc.top,
		&rc.right, &rc.bottom, &wp.showCmd, &wp.flags, &page) == 7)
	{
		SetWindowPlacement(&wp);
		if (m_bSavePage)
		{
			SetActivePage(page);
			ArrangeLayout();	// needs refresh
		}
	}
}

int CResizableSheet::GetMinWidth()
{
	int min = 0;

	// search for leftmost button
	for (int i = 0; i < 7; i++)
	{
		// left position is relative to the right border
		// of the parent window (negative value)
		if (m_szLayoutButton[i].cx < min)
			min = m_szLayoutButton[i].cx;
	}

	// sizing border width
	int border = GetSystemMetrics(SM_CXSIZEFRAME);
	
	// get tab control or wizard line left position
	CWnd* pWnd;
	CRect objrc;

	if (m_psh.dwFlags & PSH_WIZARD)
		pWnd = GetDlgItem(ID_WIZLINE);
	else
		pWnd = GetTabControl();

	pWnd->GetWindowRect(&objrc);
	ScreenToClient(&objrc);

	// add the left margin and window's border
	return -min + objrc.left + border*2;
}
