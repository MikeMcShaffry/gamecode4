// TabbedDialog.cpp : implementation file
//

#include "stdafx.h"
#include "QTabbedDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CQTabbedDialog dialog


CQTabbedDialog::CQTabbedDialog()
{
}

CQTabbedDialog::CQTabbedDialog(UINT nDlgID, CWnd* pParent /*=NULL*/)
	: CResizableDialog(nDlgID, pParent)
{
}

CQTabbedDialog::~CQTabbedDialog()
{
}

void CQTabbedDialog::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQTabbedDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CQTabbedDialog, CResizableDialog)
	//{{AFX_MSG_MAP(CQTabbedDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQTabbedDialog message handlers


//-----------------------------------------------------------------------------/
//	NAME:		CStatusMgrDlg::DeleteDataSheet()
//
//	PURPOSE:	Deletes the property sheet object.
//
//	PARAMETERS:	None
//
//	RETURNS:	None
//-----------------------------------------------------------------------------/
void CQTabbedDialog::DeletePropSheet(CPropertySheet* pSheet)
{
	TRACE("CQTabbedDialog::DeleteDataSheet()\n");
	if (pSheet != NULL)
	{
		delete pSheet;
		pSheet = NULL;
	}
}


//-----------------------------------------------------------------------------/
//	NAME:		CStatusMgrDlg::CreateDataSheet()
//
//	PURPOSE:	Creates the propertysheet object.
//
//	PARAMETERS:	int selectedPage	The page to set active when the sheet is 
//									created.  A page MUST be added to the sheet 
//									in the sheet constructor or the program will 
//									assert (or crash).
//
//	RETURNS:	None
//-----------------------------------------------------------------------------/
void CQTabbedDialog::CreatePropSheet(CPropertySheet* pSheet, UINT nFrameID, int nPageNo)
{
	// If you assert here, you tried to pass in a CPropertySheet-derived pointer 
	// instead of a CPropertySheet pointer.
	ASSERT(pSheet->IsKindOf(RUNTIME_CLASS(CPropertySheet)));

	TRACE("CQTabbedDialog::CreateDataSheet()\n");

	ASSERT_VALID(pSheet);

	CreateDlgSheet(pSheet);
	MoveDlgSheet(pSheet, nFrameID);
}


//-----------------------------------------------------------------------------/
BOOL CQTabbedDialog::CreateDlgSheet(CPropertySheet* pSheet, DWORD dwStyle/*=WS_CHILD | WS_VISIBLE*/, DWORD dwExStyle/*=0*/)
{
	TRACE("CQTabbedDialog::CreateDlgSheet()\n");
	ASSERT_VALID(pSheet);

	// 1 - Create the embedded property sheet window
	if (!pSheet->Create(this, dwStyle, dwExStyle))
	{
		TRACE0("Embedded property sheet creation failed\n");
		return FALSE;
	}

	// 2 - Add WS_TABSTOP and WS_EX_CONTROLPARENT to the property sheet styles
	pSheet->ModifyStyle(0, WS_TABSTOP);
	pSheet->ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	// 3 - Add WS_EX_CONTROLPARENT to the parent window styles
	ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	return TRUE;
}

//-----------------------------------------------------------------------------/
void CQTabbedDialog::MoveDlgSheet(CPropertySheet* pSheet, CRect rcNewPos)
{
	TRACE("CQTabbedDialog::MoveDlgSheet(1)\n");
	ASSERT_VALID(pSheet);

	// 0 - get the currently active page and set redraw to false.  This
	// will keep the dialog box from flashing when it's resized
	int nCurrentPage = pSheet->GetActiveIndex();
	pSheet->SetRedraw(FALSE);

	// 1 - Get current coordinates of tab control
	// and property sheet window
	CTabCtrl* pTabCtrl = pSheet->GetTabControl();
	ASSERT(pTabCtrl != NULL);

	CRect rcTabCtrl;
	pTabCtrl->GetWindowRect(&rcTabCtrl);
	ScreenToClient(&rcTabCtrl);

	CRect rcPSheet;
	pSheet->GetWindowRect(&rcPSheet);
	ScreenToClient(&rcPSheet);

	// 2 - Calculate margin between property sheet
	// and tab control
	int dcx = rcPSheet.Width() - rcTabCtrl.Width();
	int dcy = rcPSheet.Height() - rcTabCtrl.Height();

	// 3 - Move and resize property sheet window
	// (also moves the tab window because it is a child
	// of the property sheet window)
	pSheet->MoveWindow(rcNewPos.left, rcNewPos.top, rcNewPos.Width(), rcNewPos.Height());

	// 4 - Resize tab control window to restore
	// right / bottom margins
	pTabCtrl->SetWindowPos(NULL,
	                       0, 0,
	                       rcNewPos.Width() - dcx, rcNewPos.Height() - dcy, 
	                       SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE );

	// 5 - turn the drawing back on now, and invalidate the whole dialog
	pSheet->SetRedraw(TRUE);
	Invalidate();

	// now reset the active page
	pSheet->SetActivePage(nCurrentPage);
}

//-----------------------------------------------------------------------------/
void CQTabbedDialog::MoveDlgSheet(CPropertySheet* pSheet, UINT nIDPSheetArea)
{
	TRACE("CQTabbedDialog::MoveDlgSheet(2)\n");
	ASSERT_VALID(pSheet);

	// 1 - Retrieve property sheet destination position
	CRect rcNewPosition;
	CWnd* pWndNewArea = GetDlgItem(nIDPSheetArea);
	if (pWndNewArea == NULL)
	{
		ASSERT(FALSE);	// Invalid nIDPSheetArea
		return;
	}

	pWndNewArea->GetWindowRect(&rcNewPosition);
	ScreenToClient(&rcNewPosition);

	// 2 - Call overloaded function
	MoveDlgSheet(pSheet, rcNewPosition);
}
