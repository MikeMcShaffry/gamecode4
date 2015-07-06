/*********************************************************************
	Copyright (C) 2002 Mustafa Demirhan :)
	
	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

	1. The origin of this software must not be misrepresented; you must not
	   claim that you wrote the original software. If you use this software
	   in a product, an acknowledgment in the product documentation would be
	   appreciated but is not required.

	2. Altered source versions must be plainly marked as such, and must not be
	   misrepresented as being the original software.

	3. This notice may not be removed or altered from any source distribution.

	http://www.macroangel.com
	admin@macroangel.com
**********************************************************************/

#include "WindowEngine.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam);
bool FindText (HWND hwnd);

//CCriticalSection gWindowEngine;
CWindowEngine *pWndOp = NULL;

#define FIND_EXACT			0x0001
#define FIND_TITLE			0x0002
#define FIND_CLASSNAME		0x0004
#define FIND_TEXT			0x0008
#define FIND_ALL			0x0010
#define FIND_CASESENSITIVE	0x0020

CWindowEngine::CWindowEngine(LPCSTR lpszTitle, LPCSTR lpszClass, LPCSTR lpszText, bool bExact, bool bApplyToAll, bool bCaseSensitive)
{
	m_sTitle = lpszTitle;
	m_sClass = lpszClass;
	m_sText = lpszText;
	m_bExact = bExact;
	m_bApplyToAll = bApplyToAll;
	m_bCaseSensitive = bCaseSensitive;
	
	if (!bCaseSensitive)
	{
		m_sTitleLower = lpszTitle;
		m_sTitleLower.MakeLower ();
		m_sTextLower = lpszText;
		m_sTextLower.MakeLower ();
	}
	m_Handles.clear ();
	FindAllWindows ();
}

CWindowEngine::~CWindowEngine()
{
	m_Handles.clear ();
}

bool CWindowEngine::FocusWindow()
{
	if (m_Handles.size () <= 0)
		return false;
	
	for (list<HWND>::iterator i = m_Handles.begin (); i != m_Handles.end (); i++)
		SetForegroundWindow (*i);
	
	return true;
}

bool CWindowEngine::CloseWindow()
{
	if (m_Handles.size () <= 0)
		return false;
	
	for (list<HWND>::iterator i = m_Handles.begin (); i != m_Handles.end (); i++)
		PostMessage (*i, WM_CLOSE, 0, 0);

	return true;
}

bool CWindowEngine::MinimizeWindow()
{
	if (m_Handles.size () <= 0)
		return false;
	
	for (list<HWND>::iterator i = m_Handles.begin (); i != m_Handles.end (); i++)
		ShowWindow (*i, SW_MINIMIZE);

	return true;
}

bool CWindowEngine::MaximizeWindow()
{
	if (m_Handles.size () <= 0)
		return false;
	
	for (list<HWND>::iterator i = m_Handles.begin (); i != m_Handles.end(); i++)
		ShowWindow (*i, SW_MAXIMIZE);

	return true;
}

bool CWindowEngine::RestoreWindow()
{
	if (m_Handles.size () <= 0)
		return false;
	
	for (list<HWND>::iterator i = m_Handles.begin (); i != m_Handles.end(); i++)
		ShowWindow (*i, SW_RESTORE);
	
	return true;
}

bool CWindowEngine::HideWindow()
{
	if (m_Handles.size () <= 0)
		return false;
	
	for (list<HWND>::iterator i = m_Handles.begin (); i != m_Handles.end (); i++)
		ShowWindow (*i, SW_HIDE);

	return true;
}

bool CWindowEngine::MoveWindow(int x, int y)
{
	if (m_Handles.size () <= 0)
		return false;
	
	for (list<HWND>::iterator i = m_Handles.begin (); i != m_Handles.end (); i++)
	{
		RECT rect;
		GetWindowRect (*i, &rect);
		::MoveWindow (*i, x, y, rect.right - rect.left, rect.bottom - rect.top, TRUE);
	}
	return true;
}

bool CWindowEngine::ResizeWindow(int w, int h)
{
	if (m_Handles.size () <= 0)
		return false;
	
	for (list<HWND>::iterator i = m_Handles.begin (); i != m_Handles.end (); i++)
	{
		RECT rect;
		GetWindowRect (*i, &rect);
		::MoveWindow (*i, rect.left, rect.top, w, h, TRUE);
	}
	return true;
}

bool CWindowEngine::WaitToAppear(int nTimeout)
{
	bool bTimeout = true;
	if (nTimeout == 0)
		bTimeout = false;
	else
		bTimeout = true;
	
	int tick = GetTickCount ();
	int iEndTime = tick + nTimeout;
	
	m_Handles.clear ();
	FindAllWindows ();
	
	if (m_Handles.size () > 0)
		return true;
	
	for (;;)
	{
		tick = GetTickCount ();
		
		if (bTimeout && (tick > iEndTime))
			return false;
		
		Sleep (750);
		
		m_Handles.clear ();
		FindAllWindows ();
		
		if (m_Handles.size () > 0)
			return true;
	}
	return true;
}

bool CWindowEngine::WaitToDisappear(int nTimeout)
{
	bool bTimeout = true;
	if(nTimeout == 0)
		bTimeout = false;
	else
		bTimeout = true;
	
	int tick = GetTickCount ();
	int iEndTime = tick + nTimeout;
	
	m_Handles.clear ();
	FindAllWindows ();
	
	if (m_Handles.size () == 0)
		return true;
	
	for (;;)
	{
		tick = GetTickCount ();
		
		if (bTimeout && (tick > iEndTime))
			return false;
		
		Sleep (750);
		
		m_Handles.clear ();
		FindAllWindows ();
		
		if (m_Handles.size () == 0)
			return true;
	}
	return true;
}

void CWindowEngine::FindAllWindows()
{
	LPARAM lParam = 0;
	
	if (m_bExact)
		lParam |= FIND_EXACT;
	if (!m_sTitle.IsEmpty ())
		lParam |= FIND_TITLE;
	if (!m_sClass.IsEmpty ())
		lParam |= FIND_CLASSNAME;
	if (!m_sText.IsEmpty ())
		lParam |= FIND_TEXT;
	if (m_bApplyToAll)
		lParam |= FIND_ALL;
	if (m_bCaseSensitive)
		lParam |= FIND_CASESENSITIVE;
	
//	CSingleLock lock (&gWindowEngine);
//	lock.Lock ();
	pWndOp = this;
    EnumWindows ((WNDENUMPROC)EnumWindowsProc, lParam);
//	lock.Unlock ();
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	if (pWndOp == NULL)
		return FALSE;

	bool found = true;
	if (hwnd)
	{
		TCHAR szTemp [256];
		
		if ((lParam & FIND_TITLE))
		{
			GetWindowText (hwnd, szTemp, 255);
			CString sTitle (szTemp);
			if (!sTitle.IsEmpty ())
			{
				if ((lParam & FIND_EXACT))
				{
					if ((lParam & FIND_CASESENSITIVE))
					{
						if (sTitle.Compare (pWndOp->m_sTitle) != 0)
							found = false;
					}
					else
					{
						if (sTitle.CompareNoCase (pWndOp->m_sTitle) != 0)
							found = false;
					}
				}
				else
				{
					if ((lParam & FIND_CASESENSITIVE))
					{
						if ( sTitle.Find (pWndOp->m_sTitle, 0) == -1)
							found = false;
					}
					else
					{
						sTitle.MakeLower ();
						if ( sTitle.Find (pWndOp->m_sTitleLower, 0) == -1)
							found = false;
					}
						
				}
			}
			else
				found = false;
		}

		if (found && (lParam & FIND_CLASSNAME))
		{
			GetClassName (hwnd, szTemp, 255);
			CString sClass (szTemp);
			if (!sClass.IsEmpty ())
			{
				if ((lParam & FIND_CASESENSITIVE))
				{
					if (sClass.Compare (pWndOp->m_sClass) != 0)
						found = false;
				}
				else
				{
					if (sClass.CompareNoCase (pWndOp->m_sClass) != 0)
						found = false;
				}

			}
			else
				found = false;
		}
		
		if (found && (lParam & FIND_TEXT))
		{
			if (!FindText (hwnd))
				found = false;
		}
		
		if (found)
		{
			pWndOp->m_Handles.push_back (hwnd);
			if (!(lParam & FIND_ALL))
				return FALSE;
		}
		return TRUE;
	}
	else
		return FALSE;
}

bool FindText(HWND hwnd)
{
	if (pWndOp == NULL)
		return false;

	bool bResult = false;
	LPARAM lParam = (LPARAM)(&bResult);
	EnumChildWindows (hwnd, (WNDENUMPROC)EnumChildProc, lParam);

	return bResult;
}

BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam)
{
	if (pWndOp == NULL)
		return FALSE;
	
	if (hwnd)
	{
		TCHAR szTemp [1024] = {_T('\0')};
		
		//GetWindowText (hwnd, szTemp, 255);
		SendMessage (hwnd, WM_GETTEXT, sizeof(szTemp) / sizeof(TCHAR), (LPARAM)szTemp);
		
		CString sText (szTemp);
		if (!sText.IsEmpty ())
		{
			if (pWndOp->m_bCaseSensitive)
			{
				if (sText.Find (pWndOp->m_sText, 0) != -1)
				{
					*((bool*)lParam) = true;
					return FALSE;
				}
			}
			else
			{
				sText.MakeLower ();
				if (sText.Find (pWndOp->m_sTextLower, 0) != -1)
				{
					*((bool*)lParam) = true;
					return FALSE;
				}
				
			}
		}

		//if(FindText (hwnd))
		//{
		//	*((bool*)lParam) = true;
		//	return FALSE;
		//}
		return TRUE;
	}
	else
		return FALSE;
}