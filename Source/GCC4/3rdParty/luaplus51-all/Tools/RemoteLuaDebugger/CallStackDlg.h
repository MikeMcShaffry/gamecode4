#if !defined(AFX_CALLSTACKDLG_H__813227A7_2B88_4482_86E6_D9F5419CB0D6__INCLUDED_)
#define AFX_CALLSTACKDLG_H__813227A7_2B88_4482_86E6_D9F5419CB0D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CallStackDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCallStackDlg dialog

class CCallStackDlg : public cdxCDynamicBarDlg
{
// Construction
public:
	CCallStackDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCallStackDlg)
	enum { IDD = IDD_CALLSTACK };
	CListCtrl	m_list;
	//}}AFX_DATA

	void Update();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCallStackDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CCallStackDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDblclkCsList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CALLSTACKDLG_H__813227A7_2B88_4482_86E6_D9F5419CB0D6__INCLUDED_)
