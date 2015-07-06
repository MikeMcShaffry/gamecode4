// ChoiceWindowBtn.cpp : implementation file
//

#include "stdafx.h"
#include "ChoiceWindowBtn.h"
#include "ChoiceWindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChoiceWindowBtn

#ifdef _ITCDLL
IMPLEMENT_DYNCREATE(CChoiceWindowBtn, ITCImageButton)
#else
IMPLEMENT_DYNCREATE(CChoiceWindowBtn, CButton)
#endif

CChoiceWindowBtn::CChoiceWindowBtn()
{
	m_pChoiceWindow = new CChoiceWindow;
	m_bDisableIfEmpty = TRUE;
}

CChoiceWindowBtn::~CChoiceWindowBtn()
{
	Reset(); // Clean up
	delete m_pChoiceWindow;
}

#ifdef _ITCDLL
BEGIN_MESSAGE_MAP(CChoiceWindowBtn, ITCImageButton)
#else
BEGIN_MESSAGE_MAP(CChoiceWindowBtn, CButton)
#endif
	//{{AFX_MSG_MAP(CChoiceWindowBtn)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP

	ON_CONTROL_REFLECT(BN_CLICKED, OnButtonClicked)
	ON_NOTIFY(LVN_ITEMCHANGED, 1001, OnItemchangedListCtrl)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChoiceWindowBtn message handlers

void CChoiceWindowBtn::OnButtonClicked()
{
	CRect rcButton;

	// Size of button window
	GetWindowRect(&rcButton);

	// Create choice list window	
	m_pChoiceWindow->m_pParentBtn = this;

	// Give it a dummy height and width, so it is possible to calculate the width of the frame
	if (!::IsWindow(*m_pChoiceWindow))
	{
		VERIFY(m_pChoiceWindow->CreateEx(0, AfxRegisterWndClass(0), _T(""), WS_POPUP|WS_DLGFRAME, 
			10000, 10000, 10000, 10000, *this, (HMENU)NULL));
	}

	m_pChoiceWindow->UpdateSizeAndPosition();
	m_pChoiceWindow->ShowWindow(SW_SHOW);
}

void CChoiceWindowBtn::AddChoice(LPCTSTR szText, BOOL bChecked)
{
	// Insert new text/check pair
	CChoice* pChoice = new CChoice;
	pChoice->bChecked = bChecked;
	pChoice->strText = szText;
	m_choiceArray.Add(pChoice);
	UpdateState();
}

void CChoiceWindowBtn::Reset()
{
	// Delete all choices
	while (GetItemCount())
	{
		RemoveAt(0);
	}
}

BOOL CChoiceWindowBtn::GetCheck(int iItem)
{
	// Is the item with index iItem checked?
	ASSERT(iItem>=0 && iItem<m_choiceArray.GetSize());

	CChoice* pChoice = m_choiceArray[iItem];

	return (pChoice->bChecked);
}


void CChoiceWindowBtn::SetCheck(int nItem, BOOL bCheck)
{
	// Checks or unchecks the item at the specified index (nItem)
	ASSERT(nItem>=0 && nItem<m_choiceArray.GetSize());
	m_choiceArray[nItem]->bChecked = bCheck;	
}

void CChoiceWindowBtn::RemoveAt(int iIndex)
{
	// Remove an tem at the selected index. Make sure to deallocate its memory too
	ASSERT(iIndex>=0 && iIndex<m_choiceArray.GetSize());
	delete m_choiceArray[iIndex];
	m_choiceArray.RemoveAt(iIndex);

	UpdateState();


}

int CChoiceWindowBtn::GetItemCount()
{
	// Return the number of itemsin the list
	return m_choiceArray.GetSize();
}

CListCtrl& CChoiceWindowBtn::GetListCtrl()
{
	// Return a reference to the CListCtrl
	return m_pChoiceWindow->m_wndList;
}

void CChoiceWindowBtn::UpdateState()
{
	m_allowItemChange = false;

	// Make sure that the button is disabled if it does not have any items in it
	if (m_bDisableIfEmpty && ::IsWindow(*this))
		EnableWindow(GetItemCount());

	m_allowItemChange = true;
}

int CChoiceWindowBtn::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	m_allowItemChange = false;
#ifdef _ITCDLL
	if (ITCImageButton::OnCreate(lpCreateStruct) == -1)
		return -1;
#else	
	if (CButton::OnCreate(lpCreateStruct) == -1)
		return -1;
#endif

	// Initially disable window
	UpdateState();

	m_allowItemChange = true;
	
	return 0;
}


void CChoiceWindowBtn::OnItemchangedListCtrl(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if (m_allowItemChange)
	{
		pNMHDR->idFrom = GetDlgCtrlID();
		pNMHDR->hwndFrom = GetSafeHwnd();
		GetOwner()->SendMessage(WM_NOTIFY, pNMHDR->idFrom, (LPARAM)pNMHDR);
	}

	*pResult = 0;
}
