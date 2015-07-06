#if !defined(AFX_TABBEDDIALOG_H__CF9EC8FB_16F0_11D5_B7C2_0050DAB84384__INCLUDED_)
#define AFX_TABBEDDIALOG_H__CF9EC8FB_16F0_11D5_B7C2_0050DAB84384__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabbedDialog.h : header file
//

#include "ResizableDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CQTabbedDialog dialog

class CQTabbedDialog : public CResizableDialog
{
// Construction
public:
	CQTabbedDialog();   // default constructor
	CQTabbedDialog(UINT nDlgID, CWnd* pParent = NULL);   // standard constructor
	~CQTabbedDialog();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQTabbedDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CQTabbedDialog)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	void DeletePropSheet(CPropertySheet* pSheet);
	void CreatePropSheet(CPropertySheet* pSheet, UINT nFrameID, int nPageNo);

private:
	BOOL CreateDlgSheet (CPropertySheet* pSheet, DWORD dwStyle = WS_CHILD|WS_VISIBLE, DWORD dwExStyle = 0);
	void MoveDlgSheet   (CPropertySheet* pSheet, CRect rcNewPos);
	void MoveDlgSheet   (CPropertySheet* pSheet, UINT nIDPSheetArea);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABBEDDIALOG_H__CF9EC8FB_16F0_11D5_B7C2_0050DAB84384__INCLUDED_)
