// RemoteLuaDebugger.cpp : Defines the class behaviors for the application.
//

#include "RemoteLuaDebugger_InternalPch.h"
#include "RemoteLuaDebugger.h"

#include "MainFrm.h"
#include "RemoteLuaDebuggerDoc.h"
#include "DebuggerView.h"

#include "MainWndPlacement.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CRemoteLuaDebuggerApp

BEGIN_MESSAGE_MAP(CRemoteLuaDebuggerApp, CWinApp)
	//{{AFX_MSG_MAP(CRemoteLuaDebuggerApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRemoteLuaDebuggerApp construction

CRemoteLuaDebuggerApp::CRemoteLuaDebuggerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CRemoteLuaDebuggerApp object

CRemoteLuaDebuggerApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CRemoteLuaDebuggerApp initialization

BOOL CRemoteLuaDebuggerApp::InitInstance()
{
	AfxEnableControlContainer();

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Lua\\RemoteLuaDebugger"));

	LoadStdProfileSettings(10);  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CRemoteLuaDebuggerDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CDebuggerView));
	AddDocTemplate(pDocTemplate);

	// Get the application path.
	TCHAR moduleFileNameBuffer[_MAX_PATH];
	GetModuleFileName(GetModuleHandle(NULL), moduleFileNameBuffer, _MAX_PATH);
	m_appPath = moduleFileNameBuffer;
	int slashPos = m_appPath.ReverseFind('\\');
	m_appPath = m_appPath.Left(slashPos + 1);
	m_cachePath = m_appPath + "Cache\\";
	CreateDirectory(m_cachePath, NULL);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
//	m_pMainWnd->ShowWindow(SW_SHOW);
	CMainWndPlacement::InitialShow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CRemoteLuaDebuggerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CRemoteLuaDebuggerApp message handlers

CDebuggerView* CRemoteLuaDebuggerApp::GetDebuggerView()
{
	return ((CRemoteLuaDebuggerDoc*)((CMainFrame*)m_pMainWnd)->GetActiveDocument())->GetDebuggerView();
}


CRemoteLuaDebuggerDoc* CRemoteLuaDebuggerApp::GetDocument()
{
	return ((CRemoteLuaDebuggerDoc*)((CMainFrame*)m_pMainWnd)->GetActiveDocument());
}

static DWORD m_lastTime = 0;

BOOL CRemoteLuaDebuggerApp::OnIdle(LONG lCount) 
{
	DWORD time = timeGetTime();
	if (time - m_lastTime > 1000 / 4)
	{
		m_lastTime = time;

		GetDocument()->OnIdleCustom();
	}
	
	return CWinApp::OnIdle(lCount);
}
