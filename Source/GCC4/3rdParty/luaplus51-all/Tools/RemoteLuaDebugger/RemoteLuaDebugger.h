#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "RemoteLuaDebuggerDoc.h"
#include "MainFrm.h"

/////////////////////////////////////////////////////////////////////////////
// CRemoteLuaDebuggerApp:
// See RemoteLuaDebugger.cpp for the implementation of this class
//
class LuaDebugClient : public LuaNetworkClient
{
public:
	virtual void OnConnect();
};

class CRemoteLuaDebuggerApp : public CWinApp
{
public:
	CRemoteLuaDebuggerApp();

	CDebuggerView* GetDebuggerView();
	CRemoteLuaDebuggerDoc* GetDocument();
	CMainFrame* GetMainFrame()		{  return (CMainFrame*)m_pMainWnd;  }
	const CString& GetAppPath() const		{  return m_appPath;  }
	const CString& GetCachePath() const		{  return m_cachePath;  }
	LuaNetworkClient& GetNetworkClient()	{  return m_networkClient;  }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRemoteLuaDebuggerApp)
	public:
	virtual BOOL InitInstance();
	virtual BOOL OnIdle(LONG lCount);
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CRemoteLuaDebuggerApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	CString m_appPath;
	CString m_cachePath;
	LuaNetworkClient m_networkClient;
};

extern CRemoteLuaDebuggerApp theApp;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

