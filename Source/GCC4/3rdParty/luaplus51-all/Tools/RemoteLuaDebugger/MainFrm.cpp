// MainFrm.cpp : implementation of the CMainFrame class
//

#include "RemoteLuaDebugger_InternalPch.h"
#include "RemoteLuaDebugger.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEW_WATCHES, OnViewWatches)
	ON_COMMAND(ID_VIEW_LOCALS, OnViewLocals)
	ON_COMMAND(ID_VIEW_WINDOWLIST, OnViewWindowlist)
	ON_COMMAND(ID_VIEW_CALLSTACK, OnViewCallstack)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_VIEW_OUTPUT, OnViewOutput)
	ON_COMMAND(ID_VIEW_COMMANDLINE, OnViewCommandLine)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);

//#ifdef _SCB_REPLACE_MINIFRAME
    m_pFloatingFrameClass = RUNTIME_CLASS(CSCBMiniDockFrameWnd);
//#endif

	if (!m_localsBar.Create(_T("Local Variables"), this, CSize(200, 100),
			TRUE, AFX_IDW_CONTROLBAR_FIRST + 32))
	{ 
		TRACE0("Failed to create mybar\n");
		return -1;      // fail to create
	}

	m_localsBar.SetBarStyle(m_localsBar.GetBarStyle() |
			CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	m_localsBar.EnableDocking(CBRS_ALIGN_ANY);

	if (!m_watchBar.Create(_T("Watch Variables"), this, CSize(200, 100),
			TRUE, AFX_IDW_CONTROLBAR_FIRST + 33))
	{ 
		TRACE0("Failed to create mybar\n");
		return -1;      // fail to create
	}

	m_watchBar.SetBarStyle(m_watchBar.GetBarStyle() |
			CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	m_watchBar.EnableDocking(CBRS_ALIGN_ANY);

	EnableDocking(CBRS_ALIGN_ANY);              // <---- needed only once for the frame

	// Create the window list bar.
	if (!m_windowListBar.Create(_T("Window List"), this, CSize(200, 100),
			TRUE, AFX_IDW_CONTROLBAR_FIRST + 34))
	{ 
		TRACE0("Failed to create mybar\n");
		return -1;      // fail to create
	}
	
	m_windowListBar.SetBarStyle(m_windowListBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_windowListBar.EnableDocking(CBRS_ALIGN_ANY);

	// Create the call stack bar.
	if (!m_callStackBar.Create(_T("Call Stack"), this, CSize(200, 100),
			TRUE, AFX_IDW_CONTROLBAR_FIRST + 35))
	{ 
		TRACE0("Failed to create mybar\n");
		return -1;      // fail to create
	}
	
	m_callStackBar.SetBarStyle(m_callStackBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_callStackBar.EnableDocking(CBRS_ALIGN_ANY);

	// Create the Output bar
	if (!m_outputBar.Create(_T("Output"), this, CSize(600, 60),
			TRUE, AFX_IDW_CONTROLBAR_FIRST + 36))
	{ 
		TRACE0("Failed to create Output bar\n");
		return -1;      // fail to create
	}

	m_outputBar.SetBarStyle(m_outputBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC );
	m_outputBar.EnableDocking(CBRS_ALIGN_ANY);   // <---- needed only once for the frame

	// Create the Command bar
	if (!m_commandBar.Create(_T("Command"), this, CSize(400, 60),
			TRUE, AFX_IDW_CONTROLBAR_FIRST + 37))
	{ 
		TRACE0("Failed to create Command bar\n");
		return -1;      // fail to create
	}

	m_commandBar.SetBarStyle(m_commandBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC );
	m_commandBar.EnableDocking(CBRS_ALIGN_ANY);   // <---- needed only once for the frame

	// Dock the window list and locals bar.
	DockControlBar(&m_windowListBar, AFX_IDW_DOCKBAR_LEFT);
	DockControlBar(&m_localsBar, AFX_IDW_DOCKBAR_RIGHT);
	RecalcLayout();
	
	// Dock the call stack.
	CRect rBar;
	m_windowListBar.GetWindowRect(rBar);
	rBar.OffsetRect(0, 1);

	DockControlBar(&m_callStackBar, AFX_IDW_DOCKBAR_LEFT, rBar);
	
	// Dock the watch bar.
	m_localsBar.GetWindowRect(rBar);
	rBar.OffsetRect(0, 1);

	DockControlBar(&m_watchBar, AFX_IDW_DOCKBAR_RIGHT, rBar);

	DockControlBar(&m_wndToolBar);

	DockControlBar(&m_outputBar, AFX_IDW_DOCKBAR_BOTTOM );

	DockControlBarNextTo( &m_commandBar, &m_outputBar );

	//	ShowControlBar(&m_windowListBar, FALSE, FALSE);

	CString sProfile = _T("Windows\\BarState");
	if (VerifyBarState(sProfile))
	{
		CSizingControlBar::GlobalLoadState(this, sProfile);
		LoadBarState(sProfile);
	}
	
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


BOOL CMainFrame::DestroyWindow() 
{
	CString sProfile = _T("Windows\\BarState");
	CSizingControlBar::GlobalSaveState(this, sProfile);
	SaveBarState(sProfile);
	
	return CFrameWnd::DestroyWindow();
}


// This function is Copyright (c) 2000, Cristi Posea.
// See www.datamekanix.com for more control bars tips&tricks.
BOOL CMainFrame::VerifyBarState(LPCTSTR lpszProfileName)
{
    CDockState state;
    state.LoadState(lpszProfileName);

    for (int i = 0; i < state.m_arrBarInfo.GetSize(); i++)
    {
        CControlBarInfo* pInfo = (CControlBarInfo*)state.m_arrBarInfo[i];
        ASSERT(pInfo != NULL);
        int nDockedCount = pInfo->m_arrBarID.GetSize();
        if (nDockedCount > 0)
        {
            // dockbar
            for (int j = 0; j < nDockedCount; j++)
            {
                UINT nID = (UINT) pInfo->m_arrBarID[j];
                if (nID == 0) continue; // row separator
                if (nID > 0xFFFF)
                    nID &= 0xFFFF; // placeholder - get the ID
                if (GetControlBar(nID) == NULL)
                    return FALSE;
            }
        }
        
        if (!pInfo->m_bFloating) // floating dockbars can be created later
            if (GetControlBar(pInfo->m_nBarID) == NULL)
                return FALSE; // invalid bar ID
    }

    return TRUE;
}


void CMainFrame::DockControlBarNextTo(CControlBar* pBar, CControlBar* pTargetBar)
{
    ASSERT(pBar != NULL);
    ASSERT(pTargetBar != NULL);
    ASSERT(pBar != pTargetBar);

    // the neighbour must be already docked
    CDockBar* pDockBar = pTargetBar->m_pDockBar;
    ASSERT(pDockBar != NULL);
    UINT nDockBarID = pTargetBar->m_pDockBar->GetDlgCtrlID();
    ASSERT(nDockBarID != AFX_IDW_DOCKBAR_FLOAT);

    bool bHorz = (nDockBarID == AFX_IDW_DOCKBAR_TOP ||
        nDockBarID == AFX_IDW_DOCKBAR_BOTTOM);

    // dock normally (inserts a new row)
    DockControlBar(pBar, nDockBarID);

    // delete the new row (the bar pointer and the row end mark)
    pDockBar->m_arrBars.RemoveAt(pDockBar->m_arrBars.GetSize() - 1);
    pDockBar->m_arrBars.RemoveAt(pDockBar->m_arrBars.GetSize() - 1);

    // find the target bar
    for (int i = 0; i < pDockBar->m_arrBars.GetSize(); i++)
    {
        void* p = pDockBar->m_arrBars[i];
        if (p == pTargetBar) // and insert the new bar after it
            pDockBar->m_arrBars.InsertAt(i + 1, pBar);
    }

    // move the new bar into position
    CRect rBar;
    pTargetBar->GetWindowRect(rBar);
    rBar.OffsetRect(bHorz ? 1 : 0, bHorz ? 0 : 1);
    pBar->MoveWindow(rBar);
}

void CMainFrame::Init()
{
	m_watchBar.m_wndDlg.Init();
	m_localsBar.m_wndDlg.Init();
}

void CMainFrame::OnViewWatches() 
{
	BOOL bShow = m_watchBar.IsVisible();
	ShowControlBar(&m_watchBar, !bShow, FALSE);
}

void CMainFrame::OnViewLocals() 
{
	BOOL bShow = m_localsBar.IsVisible();
	ShowControlBar(&m_localsBar, !bShow, FALSE);
}

void CMainFrame::OnViewWindowlist() 
{
	BOOL bShow = m_windowListBar.IsVisible();
	ShowControlBar(&m_windowListBar, !bShow, FALSE);
}

void CMainFrame::OnViewCallstack() 
{
	BOOL bShow = m_callStackBar.IsVisible();
	ShowControlBar(&m_callStackBar, !bShow, FALSE);
}

void CMainFrame::OnViewOutput()
{
	BOOL bShow = m_outputBar.IsVisible();
	ShowControlBar(&m_outputBar, !bShow, FALSE);
}

void CMainFrame::OnViewCommandLine()
{
	BOOL bShow = m_commandBar.IsVisible();
	ShowControlBar(&m_commandBar, !bShow, FALSE);
}
