#pragma once
#include "afxwin.h"


// COutputDlg dialog

class COutputDlg : public cdxCDynamicBarDlg
{
public:
	COutputDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~COutputDlg();

	virtual void Append( const CString& str );
	virtual void AppendLuaCode( const CString& str );

// Dialog Data
	//{{AFX_DATA(COutputDlg)
	enum { IDD = IDD_OUTPUT };
	CEdit m_outputEdit;			// Destination for debug console output
	//}}AFX_DATA

// Overrides
protected:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COutputDlg)
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COutputDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
