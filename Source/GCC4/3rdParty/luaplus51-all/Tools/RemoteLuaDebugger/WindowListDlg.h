#if !defined(AFX_WINDOWLISTDLG_H__41E5E2DD_7C63_4380_A00F_4A550701F723__INCLUDED_)
#define AFX_WINDOWLISTDLG_H__41E5E2DD_7C63_4380_A00F_4A550701F723__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WindowListDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWindowListDlg dialog

class CWindowListDlg : public cdxCDynamicBarDlg
{
// Construction
public:
	CWindowListDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CWindowListDlg)
	enum { IDD = IDD_WINDOWLIST };
	CListCtrl	m_list;
	//}}AFX_DATA

	void Refresh();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWindowListDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CWindowListDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkWlList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WINDOWLISTDLG_H__41E5E2DD_7C63_4380_A00F_4A550701F723__INCLUDED_)
