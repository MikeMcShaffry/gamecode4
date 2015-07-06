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

#if !defined(AFX_WINDOWENGINE_H__795B8E0A_CDD5_42D1_97AD_6EBBB87C9400__INCLUDED_)
#define AFX_WINDOWENGINE_H__795B8E0A_CDD5_42D1_97AD_6EBBB87C9400__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>
#include <atlstr.h>
#pragma warning(disable: 4702)
#include <list>
using namespace std;

class CWindowEngine  
{
public:
	CWindowEngine(LPCSTR lpszTitle, LPCSTR lpszClass, LPCSTR lpszText, bool bExact, bool bApplyToAll, bool bCaseSensitive);
	virtual ~CWindowEngine();

	bool WaitToAppear(int nTimeout);
	bool WaitToDisappear(int nTimeout);

	bool MoveWindow(int x, int y);
	bool ResizeWindow(int w, int h);
	bool HideWindow();
	bool RestoreWindow();
	bool MaximizeWindow();
	bool MinimizeWindow();
	bool CloseWindow();
	bool FocusWindow();
	
	void FindAllWindows();
	
	CString m_sTitle, m_sClass, m_sText;
	CString m_sTitleLower, m_sTextLower;
	bool m_bExact, m_bApplyToAll, m_bCaseSensitive;
	
	list<HWND> m_Handles;
};

#endif // !defined(AFX_WINDOWENGINE_H__795B8E0A_CDD5_42D1_97AD_6EBBB87C9400__INCLUDED_)
