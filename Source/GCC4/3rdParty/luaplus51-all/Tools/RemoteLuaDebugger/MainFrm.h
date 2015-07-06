// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__8778910C_87AE_401C_BA40_A36FA8D0A38A__INCLUDED_)
#define AFX_MAINFRM_H__8778910C_87AE_401C_BA40_A36FA8D0A38A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VariablesBar.h"
#include "WindowListDlg.h"
#include "CallStackDlg.h"
#include "OutputDlg.h"
#include "CommandLineDlg.h"

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:
	void Init();
	CVariablesBar&	 GetLocalsBar()		{  return m_localsBar.m_wndDlg;  }
	CVariablesBar&	 GetWatchBar()		{  return m_watchBar.m_wndDlg;  }
	CWindowListDlg&	 GetWindowListBar()	{  return m_windowListBar.m_wndDlg;  }
	CCallStackDlg&	 GetCallStackBar()	{  return m_callStackBar.m_wndDlg;  }
	COutputDlg&		 GetOutputBar()		{  return m_outputBar.m_wndDlg;  }
	CCommandLineDlg& GetCommandBar()	{  return m_commandBar.m_wndDlg; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL DestroyWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	BOOL VerifyBarState(LPCTSTR lpszProfileName);
	void DockControlBarNextTo(CControlBar* pBar, CControlBar* pTargetBar);
		
protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	cdxCDynamicBarT<CVariablesBar> m_watchBar;
	cdxCDynamicBarT<CVariablesBar> m_localsBar;
	cdxCDynamicBarT<CWindowListDlg> m_windowListBar;
	cdxCDynamicBarT<CCallStackDlg> m_callStackBar;
	cdxCDynamicBarT<COutputDlg> m_outputBar;
	cdxCDynamicBarT<CCommandLineDlg> m_commandBar;
 	

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewWatches();
	afx_msg void OnViewLocals();
	afx_msg void OnViewWindowlist();
	afx_msg void OnViewCallstack();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnViewOutput();
	afx_msg void OnViewCommandLine();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__8778910C_87AE_401C_BA40_A36FA8D0A38A__INCLUDED_)
