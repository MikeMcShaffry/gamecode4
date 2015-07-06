#if !defined(AFX_VARIABLESBAR_H__E3FEBAE0_400F_40C3_87B5_A1D6260FEB45__INCLUDED_)
#define AFX_VARIABLESBAR_H__E3FEBAE0_400F_40C3_87B5_A1D6260FEB45__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VariablesBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVariablesBar dialog

class CVariablesBar : public cdxCDynamicBarDlg
{
// Construction
public:
	CVariablesBar(CWnd* pParent = NULL);   // standard constructor

	void Init();
	CVariablesGrid& GetGrid()		{  return m_grid;  }

// Dialog Data
	//{{AFX_DATA(CVariablesBar)
	enum { IDD = IDD_VARIABLES };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVariablesBar)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CVariablesGrid m_grid;

	// Generated message map functions
	//{{AFX_MSG(CVariablesBar)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VARIABLESBAR_H__E3FEBAE0_400F_40C3_87B5_A1D6260FEB45__INCLUDED_)
