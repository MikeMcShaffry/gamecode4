// MainWndPlacement.h: Schnittstelle für die Klasse CMainWndPlacement.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MainWndPlacement_H__B4F33505_79CB_11D4_86A5_2A6D82000000__INCLUDED_)
#define AFX_MainWndPlacement_H__B4F33505_79CB_11D4_86A5_2A6D82000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Subclass.h"

class CMainWndPlacement : public CSubclassWnd  
{
public:
    static void InitialShow     (UINT nCmdShow=0xFFFF)
    {
        CMainWndPlacement* pMainWndPlacement= new CMainWndPlacement(nCmdShow);
    }
private:
    CMainWndPlacement(  UINT nCmdShow=          0xFFFF,
                        CString strEntry=       _T("MainWndPos"),
                        CString strSection=     _T("Settings"))
        :   m_strEntry(strEntry),m_strSection(strSection),
            m_strFormat(_T("%u,%u,%d,%d,%d,%d,%d,%d,%d,%d")),
            m_bRestored(false)
    {
        CWnd* pWnd= AfxGetMainWnd();
        HookWindow(pWnd);
        if( nCmdShow!=0xFFFF ){
            m_bRestored= true;
            WINDOWPLACEMENT wp;
	        if (!ReadWindowPlacement(wp))
	        {
		        pWnd->ShowWindow(nCmdShow);
		        return;
	        }
            if (nCmdShow != SW_SHOWNORMAL && nCmdShow != SW_SHOW ){
		        wp.showCmd = nCmdShow;
            }
            pWnd->SetWindowPlacement(&wp);
	        pWnd->ShowWindow(wp.showCmd);
        }
    }
    virtual BOOL	RawHookWindow(HWND  hwnd)
    {
        BOOL bRes= CSubclassWnd::RawHookWindow( hwnd );
        if( !hwnd ){
            delete this;
        }
        return bRes;
    }
    bool    ReadWindowPlacement(WINDOWPLACEMENT& wp)
    {
        CString strBuffer = AfxGetApp()->GetProfileString(m_strSection, m_strEntry);
        bool bRead= !strBuffer.IsEmpty();
        if (bRead){
	        int nRead = _stscanf(strBuffer,m_strFormat ,
		        &wp.flags, &wp.showCmd,
		        &wp.ptMinPosition.x, &wp.ptMinPosition.y,
		        &wp.ptMaxPosition.x, &wp.ptMaxPosition.y,
		        &wp.rcNormalPosition.left, &wp.rcNormalPosition.top,
		        &wp.rcNormalPosition.right, &wp.rcNormalPosition.bottom);

	        if (nRead != 10)
		        bRead= false;
        }
        return bRead;
    }
    virtual LRESULT WindowProc  (UINT msg, WPARAM wp, LPARAM lp)
    {
        if (msg== WM_PAINT){
            LRESULT res= CSubclassWnd::WindowProc(msg, wp, lp);
            if( !m_bRestored ){
                m_bRestored= true;
                WINDOWPLACEMENT wp;
                if( ReadWindowPlacement(wp) ){
                    AfxGetMainWnd()->SetWindowPlacement(&wp);
                }
            }
            return res;
        }else if(msg==WM_DESTROY){
            WINDOWPLACEMENT wp;
	        wp.length = sizeof wp;
            CWnd* pWnd= AfxGetMainWnd();
	        if(pWnd->GetWindowPlacement(&wp))
	        {
		        wp.flags = 0;
		        if (pWnd->IsZoomed())
			        wp.flags |= WPF_RESTORETOMAXIMIZED;

	            TCHAR szBuffer[sizeof("-32767")*8 + sizeof("65535")*2];

	            wsprintf(szBuffer, m_strFormat,
		        wp.flags, wp.showCmd,
		        wp.ptMinPosition.x, wp.ptMinPosition.y,
		        wp.ptMaxPosition.x, wp.ptMaxPosition.y,
		        wp.rcNormalPosition.left, wp.rcNormalPosition.top,
		        wp.rcNormalPosition.right, wp.rcNormalPosition.bottom);

                AfxGetApp()->WriteProfileString(m_strSection,m_strEntry,szBuffer);
	        }
        }
        return CSubclassWnd::WindowProc(msg, wp, lp);
    }
private:
    bool    m_bRestored;
    CString m_strEntry;
    CString m_strSection;
    CString m_strFormat;
};

#endif // !defined(AFX_MainWndPlacement_H__B4F33505_79CB_11D4_86A5_2A6D82000000__INCLUDED_)
