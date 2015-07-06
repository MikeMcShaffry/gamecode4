#if !defined(AFX_CHOICEWINDOWBTN_H__58EB922F_27DF_11D3_B296_006097BEAB13__INCLUDED_)
#define AFX_CHOICEWINDOWBTN_H__58EB922F_27DF_11D3_B296_006097BEAB13__INCLUDED_

#pragma once
// ChoiceWindowBtn.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChoiceWindowBtn window

#include <afxtempl.h>

#ifdef _ITCDLL
#include <itcall.h> 
#endif

#ifdef _ITCDLL
class CChoiceWindowBtn : public ITCImageButton
#else
class CChoiceWindowBtn : public CButton
#endif
{
friend class CChoiceWindow;
	
	// Construction
	DECLARE_DYNCREATE(CChoiceWindowBtn)

public:
	void AddChoice(LPCTSTR szText, BOOL bChecked=FALSE);
	void Reset();

protected:

	class CChoice 
	{
	public:
		BOOL bChecked;
		CString strText;
	};

	CTypedPtrArray<CPtrArray, CChoice *> m_choiceArray;

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChoiceWindowBtn)
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL m_bDisableIfEmpty;
	CListCtrl& GetListCtrl();
	int GetItemCount();
	void RemoveAt(int iIndex);
	void SetCheck(int nItem, BOOL bCheck=TRUE);
	BOOL GetCheck(int iItem);
	CChoiceWindowBtn();
	virtual ~CChoiceWindowBtn();

	// Generated message map functions
protected:
	void UpdateState();
	bool m_allowItemChange;
	CChoiceWindow* m_pChoiceWindow;
	afx_msg void OnButtonClicked();
	//{{AFX_MSG(CChoiceWindowBtn)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnItemchangedListCtrl(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOICEWINDOWBTN_H__58EB922F_27DF_11D3_B296_006097BEAB13__INCLUDED_)
