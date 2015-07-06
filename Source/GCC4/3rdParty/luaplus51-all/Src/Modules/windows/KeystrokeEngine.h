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

#if !defined(AFX_KEYSTROKEENGINE_H__B7A94103_89BD_4D54_8CDA_1104DB39FC31__INCLUDED_)
#define AFX_KEYSTROKEENGINE_H__B7A94103_89BD_4D54_8CDA_1104DB39FC31__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>
#include <atlstr.h>
#pragma warning(disable: 4702)
#include <list>
using namespace std;

class CWindowEngine;
class CKeystrokeEngine  
{
public:
	CKeystrokeEngine (const CString &sKeys);
    virtual ~CKeystrokeEngine();

	void SetPause (bool bPause, int nPause);
	// if lpszTitle == NULL, then the feature is disabled
	void SetWndTitle (LPCSTR lpszTitle);
	// if lpszClass == NULL, then the feature is disabled
	void SetWndClass (LPCSTR lpszClass);
	void SetExactMatch (bool bExact);
	void SetCaseSensitive (bool bCaseSensitive);
	void SetReActivate (bool bReactivate, int nMiliseconds);
    void SetSendToWnd (bool bSendToWnd);

	bool SendKeys ();
protected:	
	bool	m_bCtrlPressed, m_bAltPressed, m_bShiftPressed;
	CString m_sKeys;
	LPCSTR  m_lpszWndTitle, m_lpszWndClass;
	bool	m_bPause, m_bSendToWnd, m_bExact, m_bCaseSensitive, m_bReactivate;
	int		m_nPause, m_nReActivate;

	enum Special_Keys
	{
		SK_None,
		SK_ShiftLock,
		SK_ShiftUnlock,
		SK_CtrlLock,
		SK_CtrlUnlock,
		SK_AltLock,
		SK_AltUnlock,
		SK_Delay,
		SK_StartRepeat,
		SK_EndRepeat
	};

	enum Key_Action
	{
		Action_KeyDown,
		Action_KeyUp,
		Action_KeyPress
	};
	
	enum KeyType
	{
		Key_NotFound,
		Key_Ignore,
		Key_Normal,
		Key_Alt,
		Key_Ctrl,
		Key_Shift,
		Key_CtrlAlt,
		Key_CtrlShift,
		Key_AltShift,
		Key_CtrlAltShift,
		Key_StartRepeat,
		Key_Delay
	};

	typedef struct tagKeyStruct
	{
		BYTE Vk;
		UINT nScan;
		Key_Action nAction;
		Special_Keys nSpecial;
		int nOptional;
	}KeyStruct;

	int GetKeyParam (const CString &sSource, CString &sDest, int nStart);
	KeyType GetNextSpecialKey(CString sKey, KeyStruct &key);
	void ParseKeys();
	bool SendSingleKey(KeyStruct &key, CWindowEngine &wnd, bool bCtrlLocked, bool bAltLocked, bool bShiftLocked);

	list <KeyStruct> m_KeyList;
public:
};

#endif // !defined(AFX_KEYSTROKEENGINE_H__B7A94103_89BD_4D54_8CDA_1104DB39FC31__INCLUDED_)
