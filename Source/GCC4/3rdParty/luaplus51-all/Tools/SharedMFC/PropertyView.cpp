/*=============================================================================
 * This is an MFC based extension class.
 * Copyright (C) 2000 YEAsoft Int'l.
 * All rights reserved.
 *=============================================================================
 * Copyright (c) 2000 YEAsoft Int'l (Leo Moll, Andrea Pennelli).
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the
 * use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *=============================================================================
 * FILENAME		:	PropertyView.cpp
 * PURPOSE		:	Implementation of a "property sheet"-like view
 * SCOPE		:	Property View Classes
 * HISTORY		: =============================================================
 * 
 * $Log: /Source/Tools/Reusable/SharedMFC/PropertyView.cpp $
 * 
 * 1     4/04/01 3:56p Joshj
 * Revision 1.8  2000/12/12 16:02:49  leo
 * Fixed wrong scrolling behaviour
 *
 * Revision 1.7  2000/12/12 15:34:08  leo
 * Added scrolling support
 *
 * Revision 1.6  2000/10/25 13:13:47  leo
 * "Fixed broken support for cut/copy/paste keystrokes"
 *
 * Revision 1.5  2000/09/28 16:15:25  leo
 * Implemented OnUpdate and OnInitialUpdate handlers
 *
 * Revision 1.4  2000/09/24 18:30:26  leo
 * Behaviour now emulates better a property sheet
 *
 * Revision 1.3  2000/09/21 16:36:37  leo
 * Made message pump more permeable
 *
 * Revision 1.2  2000/09/14 16:57:50  leo
 * Added last implementation details
 *
 * Revision 1.1  2000/09/13 17:45:57  leo
 * Property Sheet like View Class
 *
 *============================================================================*/
#include "StdAfx.h"
#include "PropertyView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*=============================================================================
 * MAKROS & DEFINES
 *============================================================================*/
#define IDC_TABCONTROL				0x7165

#define PVM_TOPPAGE_OFFSET			4
#define PVM_LEFTTAB_OFFSET			4
#define PVM_DEFLATE_TOP				2
#define PVM_DEFLATE_BOTTOM			2
#define PVM_DEFLATE_LEFT			2
#define PVM_DEFLATE_RIGHT			2

/*=============================================================================
 * PROPERTY VIEW PAGE CLASS
 *============================================================================*/
IMPLEMENT_DYNAMIC(CPropertyViewPage, baseCPropertyViewPage)

CPropertyViewPage::CPropertyViewPage ()
{
	m_lpszTemplateName	= NULL;
	m_nIDHelp			= 0;
	m_bModified			= FALSE;
	m_bFirstSetActive	= FALSE;
	m_pView				= NULL;
}

CPropertyViewPage::CPropertyViewPage (LPCTSTR lpszTemplateName, CWnd* pParentWnd)  : baseCPropertyViewPage (lpszTemplateName, pParentWnd)
{
	//{{AFX_DATA_INIT(CPropertyViewPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bModified			= FALSE;
	m_bFirstSetActive	= FALSE;
	m_pView				= NULL;
}

CPropertyViewPage::CPropertyViewPage (UINT nIDTemplate, CWnd* pParentWnd) : baseCPropertyViewPage (nIDTemplate, pParentWnd)
{
	//{{AFX_DATA_INIT(CPropertyViewPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bModified			= FALSE;
	m_bFirstSetActive	= FALSE;
	m_pView				= NULL;
}

CPropertyView * CPropertyViewPage::GetView () const
{
	ASSERT(m_pView);
	ASSERT_KINDOF(CPropertyView,m_pView);
	return m_pView;
}

void CPropertyViewPage::SetModified (BOOL bModified)
{
	if ( GetView () ) {
		GetView ()->SetModified (bModified);
	}
	m_bModified = bModified;
}

BEGIN_MESSAGE_MAP(CPropertyViewPage, baseCPropertyViewPage)
	//{{AFX_MSG_MAP(CPropertyViewPage)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CPropertyViewPage::PreTranslateMessage(MSG* pMsg) 
{
	if ( (pMsg->message == WM_KEYDOWN) && (GetAsyncKeyState (VK_CONTROL) < 0) && ((pMsg->wParam == VK_TAB) || (pMsg->wParam == VK_PRIOR) || (pMsg->wParam == VK_NEXT) || (pMsg->wParam == VK_F4) || (pMsg->wParam == VK_F6)) ) {
		// page control
		return FALSE;
	}

	return baseCPropertyViewPage::PreTranslateMessage(pMsg);
}

int CPropertyViewPage::OnCreate (LPCREATESTRUCT lpCreateStruct)
{
	lpCreateStruct->style &= ~(WS_CAPTION|WS_SYSMENU);
	if ( baseCPropertyViewPage::OnCreate (lpCreateStruct) == -1 ) {
		return -1;
	}

	m_szMin.cx = lpCreateStruct->cx;
	m_szMin.cy = lpCreateStruct->cy;

	ModifyStyle (WS_CAPTION|WS_SYSMENU, 0);
	return 0;
}

BOOL CPropertyViewPage::OnSetActive ()
{
	ASSERT_VALID(this);

	if ( m_bFirstSetActive ) {
		m_bFirstSetActive = FALSE;
	}
	else {
		UpdateData (FALSE);
	}
	return TRUE;
}

BOOL CPropertyViewPage::OnKillActive ()
{
	ASSERT_VALID(this);

	if ( !UpdateData () ) {
		TRACE0("UpdateData failed during page deactivation\n");
		return FALSE;
	}
	return TRUE;
}

BOOL CPropertyViewPage::OnApply ()
{
	ASSERT_VALID(this);

	BOOL bRet = UpdateData ();
	OnOK ();
	return bRet;
}

void CPropertyViewPage::OnOK ()
{
}

void CPropertyViewPage::OnCancel ()
{
}

/*=============================================================================
 * THE VIEW CLASS
 *============================================================================*/
IMPLEMENT_DYNCREATE(CPropertyView, CScrollView)

CPropertyView::CPropertyView()
{
	m_bAdjusted		= FALSE;
	m_bStacked		= FALSE;
	m_bScrollView	= TRUE;
	m_nActivePage	= -1;
}

CPropertyView::~CPropertyView()
{
}

int CPropertyView::GetActiveIndex () const
{
	ASSERT(m_hWnd);
	ASSERT(m_TabCtrl.m_hWnd);
	if ( !m_hWnd || !m_TabCtrl.m_hWnd ) {
		return -1;
	}
	return m_nActivePage;
}

int CPropertyView::GetPageIndex (CPropertyViewPage *pPage) const
{
	ASSERT(m_hWnd);
	ASSERT(m_TabCtrl.m_hWnd);
	ASSERT_KINDOF(CPropertyViewPage, pPage);

	if ( !m_hWnd || !m_TabCtrl.m_hWnd || !pPage ) {
		return -1;
	}

	int iCount = m_TabCtrl.GetItemCount ();
	for ( int i = 0; i < iCount; i++ ) {
		if ( GetPage (i) == pPage ) {
			return i;
		}
	}
	return -1;
}

int CPropertyView::GetPageCount () const
{
	ASSERT(m_hWnd);
	ASSERT(m_TabCtrl.m_hWnd);
	if ( !m_hWnd || !m_TabCtrl.m_hWnd ) {
		return 0;
	}
	return m_TabCtrl.GetItemCount ();
}

CPropertyViewPage * CPropertyView::GetPage (int nPage) const
{
	ASSERT(m_hWnd);
	ASSERT(m_TabCtrl.m_hWnd);
	if ( !m_hWnd || !m_TabCtrl.m_hWnd ) {
		return NULL;
	}
	if ( (nPage < 0) || (nPage >= m_TabCtrl.GetItemCount ()) ) {
		return NULL;
	}
	TCITEM tci;
	tci.mask = TCIF_PARAM;
	if ( !m_TabCtrl.GetItem (nPage, &tci) ) {
		return NULL;
	}
	return (CPropertyViewPage *) tci.lParam;
}

BOOL CPropertyView::SetActivePage (int nPage)
{
	ASSERT(m_hWnd);
	ASSERT(m_TabCtrl.m_hWnd);
	if ( !m_hWnd || !m_TabCtrl.m_hWnd ) {
		return FALSE;
	}
	if ( (nPage < 0) || (nPage >= m_TabCtrl.GetItemCount ()) ) {
		return FALSE;
	}
	if ( nPage == m_nActivePage ) {
		// nothing to do
		return TRUE;
	}
	CPropertyViewPage *pOldPage = GetPage (m_nActivePage);
	CPropertyViewPage *pNewPage = GetPage (nPage);
	if ( pOldPage && !pOldPage->OnKillActive () ) {
		// old page wants not to be switched
		return FALSE;
	}
	if ( pNewPage && !pNewPage->OnSetActive () ) {
		// new page wants not to be switched
		return FALSE;
	}

	// switch the pages
	EnablePage (pOldPage, FALSE);
	EnablePage (pNewPage, TRUE);
	// adapt the tab control
	m_TabCtrl.SetCurSel (nPage);
	m_nActivePage = nPage;
	return TRUE;
}

BOOL CPropertyView::SetActivePage (CPropertyViewPage *pPage)
{
	int iIndex = GetPageIndex (pPage);
	if ( iIndex == -1 ) {
		return FALSE;
	}
	return SetActivePage (iIndex);
}

CImageList * CPropertyView::GetImageList () const
{
	ASSERT(m_hWnd);
	ASSERT(m_TabCtrl.m_hWnd);
	if ( !m_hWnd || !m_TabCtrl.m_hWnd ) {
		return NULL;
	}
	return m_TabCtrl.GetImageList ();
}

CImageList * CPropertyView::SetImageList (CImageList *pImageList)
{
	ASSERT(m_hWnd);
	ASSERT(m_TabCtrl.m_hWnd);
	if ( !m_hWnd || !m_TabCtrl.m_hWnd ) {
		return NULL;
	}
	return m_TabCtrl.SetImageList (pImageList);
}

void CPropertyView::EnableStackedTabs (BOOL bStacked)
{
	if ( m_bStacked != bStacked ) {
		m_bStacked = bStacked;
		if ( m_hWnd && m_TabCtrl.m_hWnd ) {
			m_TabCtrl.ModifyStyle (((m_bStacked) ? (TCS_SINGLELINE) : (TCS_MULTILINE)), ((m_bStacked) ? (TCS_MULTILINE) : (TCS_SINGLELINE)));
			AdjustPagePosition (GetActivePage ());
		}
	}
}

void CPropertyView::EnableScrollView (BOOL bScrollView)
{
	if ( bScrollView != m_bScrollView ) {
		m_bScrollView = bScrollView;
		AdjustPagePosition (GetActivePage ());
	}
}

BOOL CPropertyView::AddPage (CPropertyViewPage *pPage, int nImage)
{
	ASSERT(m_hWnd);
	ASSERT(m_TabCtrl.m_hWnd);
	ASSERT(pPage);
	ASSERT_KINDOF(CPropertyViewPage, pPage);
	ASSERT(!pPage->m_hWnd);
	ASSERT(pPage->m_lpszTemplateName);

	if ( ::IsWindow (pPage->m_hWnd) ) {
		TRACE(_T("View Page already created\n"));
		return FALSE;
	}

	pPage->m_pView				= this;
	pPage->m_bFirstSetActive	= FALSE;

	if ( !pPage->Create (pPage->m_lpszTemplateName, &m_TabCtrl) ) {
		TRACE(_T("Failed to create view page\n"));
		pPage->m_pView = NULL;
		return FALSE;
	}

	CString		csName;
	TC_ITEM		tcItem;
	pPage->GetWindowText (csName);

	tcItem.mask		= TCIF_TEXT | TCIF_PARAM | ((nImage != -1) ? (TCIF_IMAGE) : (0));
	tcItem.pszText	= (LPTSTR) (LPCTSTR) csName;
	tcItem.lParam	= (LPARAM) pPage;
	tcItem.iImage	= ((nImage != -1) ? (nImage) : (0));

	if ( m_TabCtrl.InsertItem (m_TabCtrl.GetItemCount (), &tcItem) == -1 ) {
		TRACE(_T("Failed to add the property page\n"));
		pPage->DestroyWindow ();
		pPage->m_hWnd = NULL;
		return FALSE;
	}


	if ( m_bAdjusted ) {
		// view is up and running. resize properly
		CRect rect; GetClientRect (&rect);
		OnSize (SIZE_RESTORED, rect.Width (), rect.Height ());
	}

	if ( m_nActivePage == -1 ) {
		// first page. enable it and select it
		m_TabCtrl.SetCurSel (0);
		m_nActivePage = 0;
		pPage->m_bFirstSetActive = TRUE;
		pPage->OnSetActive ();	// ignore result
		EnablePage (pPage, TRUE);
	}
	else {
		// other page is visible. disable it
		EnablePage (pPage, FALSE);
	}
	return TRUE;
}

BOOL CPropertyView::RemovePage (CPropertyViewPage *pPage)
{
	int nPage = GetPageIndex (pPage);
	if ( nPage == -1 ) {
		return FALSE;
	}
	return RemovePage (pPage, nPage);
}

BOOL CPropertyView::RemovePage (int nPage)
{
	CPropertyViewPage *pPage = GetPage (nPage);
	if ( !pPage ) {
		return FALSE;
	}
	return RemovePage (pPage, nPage);
}

BOOL CPropertyView::RemovePage (CPropertyViewPage *pPage, int iIndex)
{
	if ( iIndex == m_nActivePage ) {
		// page to delete is the active page
		pPage->OnKillActive ();	// ignore result
		pPage->DestroyWindow ();
		m_TabCtrl.DeleteItem (iIndex);
		m_nActivePage = -1; // no page active
		if ( m_TabCtrl.GetItemCount () ) {
			// switch control to the another page
			SetActivePage ((iIndex) ? (iIndex - 1) : (m_TabCtrl.GetItemCount () - 1));
		}
	}
	else {
		// page to delete is not the active page
		pPage->DestroyWindow ();
		m_TabCtrl.DeleteItem (iIndex);
	}
	return TRUE;
}

BOOL CPropertyView::Apply ()
{
	int iItems = m_TabCtrl.GetItemCount ();
	for ( int i = 0; i < iItems; i++ ) {
		CPropertyViewPage *pPage = GetPage (i);
		if ( pPage && !pPage->OnApply () ) {
			return FALSE;
		}
	}
	return TRUE;
}

void CPropertyView::SetModified (BOOL bChanged)
{
	if ( GetDocument () ) {
		GetDocument ()->SetModifiedFlag (bChanged);
	}
}

void CPropertyView::EnablePage (CPropertyViewPage *pPage, BOOL bEnable)
{
	if ( !pPage || !pPage->m_hWnd ) {
		return;
	}
	pPage->ShowWindow ((bEnable) ? (SW_SHOW) : (SW_HIDE));
	pPage->EnableWindow (bEnable);
	if ( bEnable ) {
		pPage->SetFocus ();
		AdjustPagePosition (pPage);
	}
}


void CPropertyView::AdjustPagePosition (CPropertyViewPage *pPage)
{
	if ( !pPage || !pPage->m_hWnd ) {
		return;
	}

	BOOL bTabOnBottom = (m_TabCtrl.GetStyle () & TCS_BOTTOM) != 0;

	// calculate needed client area
	CRect rect(0, 0, pPage->m_szMin.cx + PVM_DEFLATE_LEFT + PVM_DEFLATE_RIGHT, pPage->m_szMin.cy + PVM_DEFLATE_TOP + PVM_DEFLATE_BOTTOM);
	CSize szScroll;
	CSize szReal;
	CRect rcReal;

	GetClientRect (rcReal);
	int cx = rcReal.Width ();
	int cy = rcReal.Height ();
	szScroll = CSize(cx,cy);

	ScrollToPosition (CPoint (0,0));

	if ( m_bScrollView ) {
		m_TabCtrl.AdjustRect (TRUE, &rect);
		szReal.cx = (rect.right - rect.left);
		szReal.cy = (rect.bottom - rect.top) + PVM_TOPPAGE_OFFSET;
			
		szScroll.cx = (szReal.cx > cx) ? (szReal.cx) : (cx);
		szScroll.cy = (szReal.cy > cy) ? (szReal.cy) : (cy);

		SetScrollSizes (MM_TEXT, szScroll);

		// now compute the rect for the tab control:
		GetClientRect (rcReal); 

		rect.left	= 0;
		rect.top	= (bTabOnBottom) ? (0) : (PVM_TOPPAGE_OFFSET);
		rect.right	= (szReal.cx > rcReal.Width ()) ? (szReal.cx) : (rcReal.Width ());
		rect.bottom	= (szReal.cy > rcReal.Height ()) ? (szReal.cy) : (rcReal.Height ());
	}
	else {
		SetScrollSizes (MM_TEXT, szScroll);
		rect = CRect(0, 0, cx, cy);
	}

	// reposition the tab control.
	m_TabCtrl.MoveWindow (&rect);
	m_TabCtrl.AdjustRect (FALSE, &rect);
	rect.DeflateRect (PVM_DEFLATE_LEFT, PVM_DEFLATE_TOP, PVM_DEFLATE_RIGHT, PVM_DEFLATE_BOTTOM);
	pPage->MoveWindow (&rect);
}

BEGIN_MESSAGE_MAP(CPropertyView, CScrollView)
	//{{AFX_MSG_MAP(CPropertyView)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
	ON_NOTIFY(TCN_SELCHANGING, IDC_TABCONTROL, OnTabChanging)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TABCONTROL, OnTabChanged)
END_MESSAGE_MAP()

BOOL CPropertyView::PreTranslateMessage(MSG* pMsg) 
{
	ASSERT_VALID(this);
	ASSERT(m_hWnd);
	ASSERT(m_TabCtrl.m_hWnd);

	// allow tooltip messages to be filtered
	if ( CScrollView::PreTranslateMessage(pMsg) ) {
		return TRUE;
	}
	if ( !m_hWnd || !m_TabCtrl.m_hWnd || !m_TabCtrl.GetItemCount () ) {
		return FALSE;
	}

	// allow view to translate Ctrl+Tab, Shift+Ctrl+Tab,
	//  Ctrl+PageUp, and Ctrl+PageDown
	if ( (pMsg->message == WM_KEYDOWN) && (GetAsyncKeyState (VK_CONTROL) < 0) ) {
		int nNewPage;

		switch ( pMsg->wParam ) {
		case VK_TAB:
			if ( GetAsyncKeyState (VK_SHIFT) < 0 ) {
				nNewPage = (m_nActivePage) ? (m_nActivePage - 1) : (m_TabCtrl.GetItemCount () - 1);
			}
			else {
				nNewPage = (m_nActivePage == (m_TabCtrl.GetItemCount () - 1)) ? (0) : (m_nActivePage + 1);
			}
			break;
		case VK_NEXT:
			nNewPage = (m_nActivePage == (m_TabCtrl.GetItemCount () - 1)) ? (0) : (m_nActivePage + 1);
			break;
		case VK_PRIOR:
			nNewPage = (m_nActivePage) ? (m_nActivePage - 1) : (m_TabCtrl.GetItemCount () - 1);
			break;
		default:
			return FALSE;
		}
		SetActivePage (nNewPage);
		return TRUE;
	}
	return FALSE;
}

void CPropertyView::OnDraw(CDC* pDC)
{
}

/////////////////////////////////////////////////////////////////////////////
// CPropertyView diagnostics

#ifdef _DEBUG
void CPropertyView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CPropertyView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPropertyView message handlers

int CPropertyView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CScrollView::OnCreate(lpCreateStruct) == -1 ) {
		return -1;
	}

	SetScrollSizes (MM_TEXT, CSize(1000,1000));

	DWORD	dwStyle = TCS_FORCELABELLEFT | TCS_FOCUSNEVER | ((m_bStacked) ? (TCS_MULTILINE) : (TCS_SINGLELINE));
	CRect	rect; GetClientRect (&rect);
	if ( rect.bottom && rect.right ) {
		rect.top = PVM_TOPPAGE_OFFSET;
	}

	PreCreateTabControl (rect, dwStyle);

	dwStyle &= 0x0000ffff;
	dwStyle |= WS_CHILD | WS_VISIBLE;
	
	if ( !m_TabCtrl.Create (dwStyle, rect, this, IDC_TABCONTROL) ) {
		TRACE("Failed to create Tab control\n");
		return -1;
	}
	m_TabCtrl.SetFont (CFont::FromHandle ((HFONT) GetStockObject (DEFAULT_GUI_FONT)));
	
	return 0;
}

BOOL CPropertyView::OnEraseBkgnd(CDC* pDC) 
{
	BOOL bTabOnBottom = (m_TabCtrl.GetStyle () & TCS_BOTTOM) != 0;

	CRect	rect; GetClientRect (&rect);
	if ( bTabOnBottom ) {
		rect.top = rect.bottom - PVM_TOPPAGE_OFFSET;
	}
	else {
		rect.bottom = rect.top + PVM_TOPPAGE_OFFSET;
	}
	pDC->FillSolidRect (&rect, GetSysColor (COLOR_3DFACE));
	return TRUE;
}

void CPropertyView::OnSize(UINT nType, int cx, int cy) 
{
	BOOL bTabOnBottom = (m_TabCtrl.GetStyle () & TCS_BOTTOM) != 0;


	CPropertyViewPage *pPage = GetActivePage ();
	if ( !pPage ) {
		// no page...
		SetScrollSizes (MM_TEXT, CSize(cx,cy));
		CScrollView::OnSize(nType, cx, cy);
		m_TabCtrl.MoveWindow (0, (bTabOnBottom) ? (0) : (PVM_TOPPAGE_OFFSET), cx, cy - PVM_TOPPAGE_OFFSET);
		return;
	}

	// calculate needed client area
	CRect	rect(0, 0, pPage->m_szMin.cx + PVM_DEFLATE_LEFT + PVM_DEFLATE_RIGHT, pPage->m_szMin.cy + PVM_DEFLATE_TOP + PVM_DEFLATE_BOTTOM);
	CSize	szScroll(cx,cy);
	CSize	szReal;
	CPoint	ptShift(0,0);

	if ( m_bScrollView ) {
		m_TabCtrl.AdjustRect (TRUE, &rect);
		szReal.cx = (rect.right - rect.left);
		szReal.cy = (rect.bottom - rect.top) + PVM_TOPPAGE_OFFSET;
			
		szScroll.cx = (szReal.cx > cx) ? (szReal.cx) : (cx);
		szScroll.cy = (szReal.cy > cy) ? (szReal.cy) : (cy);

		SetScrollSizes (MM_TEXT, szScroll);
		CScrollView::OnSize(nType, cx, cy);

		// now compute the rect for the tab control:
		CRect rcReal; GetClientRect (rcReal); 

		rect.left	= 0;
		rect.top	= (bTabOnBottom) ? (0) : (PVM_TOPPAGE_OFFSET);
		rect.right	= (szReal.cx > rcReal.Width ()) ? (szReal.cx) : (rcReal.Width ());
		rect.bottom	= (szReal.cy > rcReal.Height ()) ? (szReal.cy) : (rcReal.Height ());

		ptShift = GetScrollPosition ();
		rect.OffsetRect (-ptShift.x, -ptShift.y);
	}
	else {
		SetScrollSizes (MM_TEXT, szScroll);
		CScrollView::OnSize(nType, cx, cy);
		rect = CRect(0, 0, cx, cy);
	}

	// reposition the tab control.
	m_TabCtrl.MoveWindow (&rect);
	m_TabCtrl.AdjustRect (FALSE, &rect);
	rect.DeflateRect (PVM_DEFLATE_LEFT, PVM_DEFLATE_TOP, PVM_DEFLATE_RIGHT, PVM_DEFLATE_BOTTOM);

	rect.OffsetRect (ptShift);
	pPage->MoveWindow (&rect);

	if ( !m_bAdjusted && m_TabCtrl.GetItemCount () ) {
		// one time adjustment
		m_bAdjusted = TRUE;
		pPage->BringWindowToTop ();
	}
}

void CPropertyView::OnInitialUpdate ()
{
	for ( int i = 0; i < m_TabCtrl.GetItemCount (); i++ ) {
		CPropertyViewPage *pPage = GetPage (i);
		if ( pPage ) {
			pPage->OnInitialUpdate ();
		}
	}
}

void CPropertyView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	for ( int i = 0; i < m_TabCtrl.GetItemCount (); i++ ) {
		CPropertyViewPage *pPage = GetPage (i);
		if ( pPage ) {
			pPage->OnUpdate (pSender, lHint, pHint);
		}
	}
}

void CPropertyView::OnSetFocus(CWnd* pOldWnd) 
{
	CScrollView::OnSetFocus(pOldWnd);
	CPropertyViewPage *pPage = GetActivePage ();
	if ( pPage ) {
		pPage->SetFocus ();
	}
}

void CPropertyView::OnTabChanging (NMHDR * pNM, LRESULT * pResult)
{
	*pResult = FALSE;

	int nPage = m_TabCtrl.GetCurSel ();
	ASSERT(nPage != -1);
	if ( nPage == -1 ) {
		// SNH: No Selection - Control is empty
		return;
	}

	CPropertyViewPage *pPage = GetPage (nPage);
	if ( pPage && !pPage->OnKillActive () ) {
		// do not change the selected tab
		*pResult = TRUE;
	}
}

void CPropertyView::OnTabChanged (NMHDR * pNM, LRESULT * pResult)
{
	int nPage = m_TabCtrl.GetCurSel ();
	ASSERT(nPage != -1);
	if ( nPage == -1 ) {
		// SNH: No Selection - Control is empty
		return;
	}

	CPropertyViewPage *pOldPage = GetPage (m_nActivePage);
	CPropertyViewPage *pNewPage = GetPage (nPage);
	if ( !pNewPage || !pNewPage->OnSetActive () ) {
		// do nothing and return to the previous selection
		m_TabCtrl.SetCurSel (m_nActivePage);
		return;
	}

	// switch the pages
	EnablePage (pOldPage, FALSE);
	EnablePage (pNewPage, TRUE);
	m_nActivePage = nPage;
	*pResult = TRUE;
}

//
// EoF
////////
