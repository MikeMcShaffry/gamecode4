#pragma once

#include "ACEditCtrl.h"

// CCommandLineInputEdit edit control
class CCommandLineInputEdit
	: public CACEditCtrl
{
public:
	//{{AFX_MSG(CCommandLineInputEdit)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg UINT OnGetDlgCode();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};


// CCommandLineDlg dialog

class CCommandLineDlg : public cdxCDynamicBarDlg
{
public:
	CCommandLineDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCommandLineDlg();

// Dialog Data
	//{{AFX_DATA(CCommandLineDlg)
	enum { IDD = IDD_COMMAND_LINE };
	CCommandLineInputEdit m_editCommand;	// Command line input box
	//}}AFX_DATA

// Overrides
protected:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCommandLineDlg)
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CCommandLineDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
