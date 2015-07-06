#if !defined(AFX_CHOICEWINDOW_H__58EB9233_27DF_11D3_B296_006097BEAB13__INCLUDED_)
#define AFX_CHOICEWINDOW_H__58EB9233_27DF_11D3_B296_006097BEAB13__INCLUDED_

#pragma once
// ChoiceWindow.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChoiceWindow window


class CChoiceWindowBtn;

class CChoiceWindow : public CWnd
{
friend class CChoiceWindowBtn;	
// Construction
public:
	CChoiceWindow();

// Attributes
public:
	CListCtrl m_wndList;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChoiceWindow)
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	CChoiceWindowBtn* m_pParentBtn;
	virtual ~CChoiceWindow();

	// Generated message map functions
protected:
	void UpdateSizeAndPosition();
	void HideDropDown();
	afx_msg void OnKillFocusListView(NMHDR* pNMHDR, LRESULT* pResult);
	//{{AFX_MSG(CChoiceWindow)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	virtual void OnListviewKeydown(NMHDR* pnmhdr, LRESULT* pResult);
	afx_msg void OnItemChangedListView(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOICEWINDOW_H__58EB9233_27DF_11D3_B296_006097BEAB13__INCLUDED_)
