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

#include "KeystrokeEngine.h"
#include "WindowEngine.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define MAX_SK_LEN 20		// Maximum length of a speical key description

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CKeystrokeEngine::CKeystrokeEngine(const CString &sKeys) :
	m_bCtrlPressed (false), m_bAltPressed (false), m_bShiftPressed (false),
    m_bPause (false), m_bSendToWnd (false), m_bExact (false), m_bCaseSensitive (false),
	m_bReactivate (false), m_nPause (0), m_nReActivate (0), 
	m_lpszWndTitle (NULL), m_lpszWndClass (NULL)
{
	m_sKeys = sKeys;
	ParseKeys ();
}

CKeystrokeEngine::~CKeystrokeEngine()
{
	// release any remaining down keys...
	keybd_event(VK_CONTROL, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	keybd_event(VK_MENU, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	keybd_event(VK_SHIFT, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
}

// Sends all keys in the m_KeyList
bool CKeystrokeEngine::SendKeys()
{
	// TODO: KEYEVENTF_SCANCODE a bak
	// TODO: Reset keyboard state before sending keys
	// TODO: LOCK keyboard while sending keystrokes. 
	//       Dunno how to do, but seems to be a nice feature
	bool bShiftLocked = false, bAltLocked = false, bCtrlLocked = false;
	
	// Focus window if necessary
	CWindowEngine wnd (m_lpszWndTitle, m_lpszWndClass, NULL, m_bExact, true, m_bCaseSensitive);
	if (m_bSendToWnd)
	{
		wnd.FindAllWindows ();
		if (wnd.m_Handles.size () > 0)
		{
			bool bFoundInForeground = false;
			for(list<HWND>::iterator i = wnd.m_Handles.begin(); i != wnd.m_Handles.end(); i++)
			{
				if (GetForegroundWindow () == *i)
				{
					bFoundInForeground = true;
					break;
				}
			}
			if (!bFoundInForeground)
				wnd.FocusWindow ();
		}
		// if window is not found and the keys will be sent only that window
		// then return false
		else
			return false;
	}

	for(list<KeyStruct>::iterator i = m_KeyList.begin (); i != m_KeyList.end (); i++)
	{
		// handle special keys
		switch ((*i).nSpecial)
		{
		case SK_None:
			if (!SendSingleKey (*i, wnd, bCtrlLocked, bAltLocked, bShiftLocked))
				return false;
			break;
		case SK_ShiftLock:
			bShiftLocked = true;
			if (!SendSingleKey (*i, wnd, bCtrlLocked, bAltLocked, bShiftLocked))
				return false;
			break;
		case SK_ShiftUnlock:
			bShiftLocked = false;
			if (!SendSingleKey (*i, wnd, bCtrlLocked, bAltLocked, bShiftLocked))
				return false;
			break;
		case SK_CtrlLock:
			bCtrlLocked = true;
			if (!SendSingleKey (*i, wnd, bCtrlLocked, bAltLocked, bShiftLocked))
				return false;
			break;
		case SK_CtrlUnlock:
			bCtrlLocked = false;
			if (!SendSingleKey (*i, wnd, bCtrlLocked, bAltLocked, bShiftLocked))
				return false;
			break;
		case SK_AltLock:
			bAltLocked = true;
			if (!SendSingleKey (*i, wnd, bCtrlLocked, bAltLocked, bShiftLocked))
				return false;
			break;
		case SK_AltUnlock:
			bAltLocked = false;
			if (!SendSingleKey (*i, wnd, bCtrlLocked, bAltLocked, bShiftLocked))
				return false;
			break;
		case SK_Delay:
			if ((*i).nOptional > 0)
				Sleep ((*i).nOptional);
			break;
		// the hardest and longest part! Get all the keys within 
		// <REPEAT n> and <END_REPEAT> blocks and put them 
		// into a list and then send them n times
		case SK_StartRepeat:
			bool bContinueLoop = true;
			int nRepeat = (*i).nOptional;
			if (nRepeat <= 1)
				break;
			
			++ i;
			list<KeyStruct> tempList;
			while (i != m_KeyList.end () && bContinueLoop)
			{
				switch ((*i).nSpecial)
				{
					case SK_None:
					case SK_ShiftLock:
					case SK_ShiftUnlock:
					case SK_CtrlLock:
					case SK_CtrlUnlock:
					case SK_AltLock:
					case SK_AltUnlock:
					case SK_Delay:
						tempList.push_back (*i);
						++ i;
						break;
					// Dont event think of this! Nested repeats are not supported
					//case SK_StartRepeat:
					//	break;
					case SK_EndRepeat:
						bContinueLoop = false;
						break;
					default:
						bContinueLoop = false;
						break;
				}
			}
			// OK. Now send all buffered keys nRepeat times.
			for (int idx = 0; idx < nRepeat; idx++)
			{
				for(list<KeyStruct>::iterator it2 = tempList.begin (); it2 != tempList.end (); it2++)
				{
					switch ((*it2).nSpecial)
					{
						case SK_None:
							if (!SendSingleKey (*it2, wnd, bCtrlLocked, bAltLocked, bShiftLocked))
								return false;
							break;
						case SK_ShiftLock:
							bShiftLocked = true;
							if (!SendSingleKey (*it2, wnd, bCtrlLocked, bAltLocked, bShiftLocked))
								return false;
							break;
						case SK_ShiftUnlock:
							bShiftLocked = false;
							if (!SendSingleKey (*it2, wnd, bCtrlLocked, bAltLocked, bShiftLocked))
								return false;
							break;
						case SK_CtrlLock:
							bCtrlLocked = true;
							if (!SendSingleKey (*it2, wnd, bCtrlLocked, bAltLocked, bShiftLocked))
								return false;
							break;
						case SK_CtrlUnlock:
							bCtrlLocked = false;
							if (!SendSingleKey (*it2, wnd, bCtrlLocked, bAltLocked, bShiftLocked))
								return false;
							break;
						case SK_AltLock:
							bAltLocked = true;
							if (!SendSingleKey (*it2, wnd, bCtrlLocked, bAltLocked, bShiftLocked))
								return false;
							break;
						case SK_AltUnlock:
							bAltLocked = false;
							if (!SendSingleKey (*it2, wnd, bCtrlLocked, bAltLocked, bShiftLocked))
								return false;
							break;
						case SK_Delay:
							if ((*it2).nOptional > 0)
								Sleep ((*it2).nOptional);
							break;
					}
				}
			}
			// if i == end () this means that the <END_REPEAT> is missing
			// in order to prevent an infinite loop, just exit the function!
			if (i == m_KeyList.end ())
				return true;
			break;
		//case SK_EndRepeat:
		//	break;
		}
	}
	return true;
}

// Sends a single key. bCtrlLocked, bAltLocked, bShiftLocked are set 
// to true if any of these keys are locked. If these keys are locked
// then dont send any KEY_UP messages for theses keys. Otherwise, they
// will be unlocked
bool CKeystrokeEngine::SendSingleKey(KeyStruct &key, CWindowEngine &wnd, bool bCtrlLocked, bool bAltLocked, bool bShiftLocked)
{
	// First focus the window if needed...
	if (m_bSendToWnd)
	{
		wnd.FindAllWindows ();
		if (wnd.m_Handles.size () > 0)
		{
			bool bFoundInForeground = false;
			for(list<HWND>::iterator i = wnd.m_Handles.begin(); i != wnd.m_Handles.end(); i++)
			{
				if (GetForegroundWindow () == *i)
				{
					bFoundInForeground = true;
					break;
				}
			}
			if (!bFoundInForeground)
			{
				if (m_bReactivate)
				{
					Sleep (m_nReActivate * 1000);
					wnd.FocusWindow ();
				}
				else
					return false;
			}
		}
		else
			return false;
	}
	if (key.nSpecial != SK_Delay && key.nSpecial != SK_StartRepeat && key.nSpecial != SK_EndRepeat)
	{
		bool bDoIt = true;
		if (key.nSpecial == SK_None)
		{
			if (	(key.Vk == VK_CONTROL && key.nAction == Action_KeyUp && bCtrlLocked) ||
					(key.Vk == VK_MENU && key.nAction == Action_KeyUp && bAltLocked) ||
					(key.Vk == VK_SHIFT && key.nAction == Action_KeyUp && bShiftLocked) )
				bDoIt = false;
		}
		if (bDoIt)
		{
			if (key.nAction == Action_KeyDown || key.nAction == Action_KeyPress)
				keybd_event(key.Vk, (BYTE)key.nScan, KEYEVENTF_EXTENDEDKEY | 0, 0 );
			if (key.nAction == Action_KeyUp || key.nAction == Action_KeyPress)
				keybd_event(key.Vk, (BYTE)key.nScan, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
		}
	}

	if (m_bPause)
		Sleep (m_nPause);

	return true;
}

// Parses the keys in m_sKeys string and puts them into m_KeyList
void CKeystrokeEngine::ParseKeys()
{
	KeyStruct	tempKey, keyAltHold, keyCtrlHold, keyShiftHold,
				keyAltRelease, keyCtrlRelease, keyShiftRelease;

	keyAltHold.nAction = Action_KeyDown;
	keyAltHold.nOptional = 0;
	keyAltHold.nSpecial = SK_None;
	keyAltHold.Vk = VK_MENU;
	keyAltHold.nScan = MapVirtualKey (VK_MENU, 0);

	keyCtrlHold.nAction = Action_KeyDown;
	keyCtrlHold.nOptional = 0;
	keyCtrlHold.nSpecial = SK_None;
	keyCtrlHold.Vk = VK_CONTROL;
	keyCtrlHold.nScan = MapVirtualKey (VK_CONTROL, 0);

	keyShiftHold.nAction = Action_KeyDown;
	keyShiftHold.nOptional = 0;
	keyShiftHold.nSpecial = SK_None;
	keyShiftHold.Vk = VK_SHIFT;
	keyShiftHold.nScan = MapVirtualKey (VK_SHIFT, 0);

	keyAltRelease.nAction = Action_KeyUp;
	keyAltRelease.nOptional = 0;
	keyAltRelease.nSpecial = SK_None;
	keyAltRelease.Vk = VK_MENU;
	keyAltRelease.nScan = MapVirtualKey (VK_MENU, 0);

	keyCtrlRelease.nAction = Action_KeyUp;
	keyCtrlRelease.nOptional = 0;
	keyCtrlRelease.nSpecial = SK_None;
	keyCtrlRelease.Vk = VK_CONTROL;
	keyCtrlRelease.nScan = MapVirtualKey (VK_CONTROL, 0);

	keyShiftRelease.nAction = Action_KeyUp;
	keyShiftRelease.nOptional = 0;
	keyShiftRelease.nSpecial = SK_None;
	keyShiftRelease.Vk = VK_SHIFT;
	keyShiftRelease.nScan = MapVirtualKey (VK_SHIFT, 0);

	m_KeyList.clear ();
	int idx = 0;
	int nKeyLength = m_sKeys.GetLength ();
	char c = 0;
	char szTemp [MAX_SK_LEN];
	SHORT nTemp;
	bool bNotASpecialKey = false;
	
	bool bPushAltRelease = false, bPushCtrlRelease = false, bPushShiftRelease = false;

	while (idx < nKeyLength)
	{
		c = m_sKeys.GetAt (idx);
		// if a normal key
		if (bNotASpecialKey || 
			((c != '<') || ((c == '<') && (idx == nKeyLength - 1)))
			)
		{
			bNotASpecialKey = false;
			tempKey.nAction = Action_KeyPress;
			tempKey.nSpecial = SK_None;
			nTemp = VkKeyScan (c);
			tempKey.Vk = LOBYTE (nTemp);
            int nTempState = HIBYTE(nTemp);

			if (nTempState & 0x0001)
				m_KeyList.push_back (keyShiftHold);
			if (nTempState & 0x0002)
				m_KeyList.push_back (keyCtrlHold);
			if (nTempState & 0x0004)
				m_KeyList.push_back (keyAltHold);
			
			tempKey.nScan = MapVirtualKey (tempKey.Vk, 0);
			m_KeyList.push_back (tempKey);

			if (nTempState & 0x0004)
				m_KeyList.push_back (keyAltRelease);
			if (nTempState & 0x0002)
				m_KeyList.push_back (keyCtrlRelease);
			if (nTempState & 0x0001)
				m_KeyList.push_back (keyShiftRelease);

			if (bPushAltRelease)
				m_KeyList.push_back (keyAltRelease);
			if (bPushCtrlRelease)
				m_KeyList.push_back (keyCtrlRelease);
			if (bPushShiftRelease)
				m_KeyList.push_back (keyShiftRelease);
			bPushAltRelease = false;
			bPushCtrlRelease = false;
			bPushShiftRelease = false;

			++ idx;
		}
		// if a special key
		else
		{
			bNotASpecialKey = false;
			int nTempIdx = idx + 1;
			int nOffset = 0;
			while (nTempIdx < nKeyLength && (nTempIdx < nTempIdx + MAX_SK_LEN - 1))
			{
				c = m_sKeys.GetAt (nTempIdx);
				// found the first token
				if (c == ' ' || c == '>')
				{
					++nTempIdx;
					szTemp [nOffset] = '\0';
					KeyType nType = GetNextSpecialKey (szTemp, tempKey);
					if (nType != Key_StartRepeat && nType != Key_Delay)
					{
						if (c == ' ')
						{
							CString sDest;
							int i = GetKeyParam (m_sKeys, sDest, nTempIdx);
							if (i > 0)
                                nTempIdx += i;
							else
							{
								bNotASpecialKey = true;
								break;
							}
						}
					}
					if (nType != Key_NotFound && nType != Key_Normal)
					{
						if (bPushAltRelease)
							m_KeyList.push_back (keyAltRelease);
						if (bPushCtrlRelease)
							m_KeyList.push_back (keyCtrlRelease);
						if (bPushShiftRelease)
							m_KeyList.push_back (keyShiftRelease);
					}
					switch (nType)
					{
					case Key_NotFound:
						bNotASpecialKey = true;
						break;
					case Key_Ignore:
						bPushAltRelease = false;
						bPushCtrlRelease = false;
						bPushShiftRelease = false;
						break;
					case Key_Normal:
						m_KeyList.push_back (tempKey);
						if (bPushAltRelease)
							m_KeyList.push_back (keyAltRelease);
						if (bPushCtrlRelease)
							m_KeyList.push_back (keyCtrlRelease);
						if (bPushShiftRelease)
							m_KeyList.push_back (keyShiftRelease);
						bPushAltRelease = false;
						bPushCtrlRelease = false;
						bPushShiftRelease = false;
						break;
					case Key_Alt:
						m_KeyList.push_back (keyAltHold);

						bPushAltRelease = true;
						bPushCtrlRelease = false;
						bPushShiftRelease = false;
						break;
					case Key_Ctrl:
						m_KeyList.push_back (keyCtrlHold);

						bPushCtrlRelease = true;
						bPushAltRelease = false;
						bPushShiftRelease = false;
						break;
					case Key_Shift:
						m_KeyList.push_back (keyShiftHold);
						bPushShiftRelease = true;
						bPushAltRelease = false;
						bPushCtrlRelease = false;
						break;
					case Key_CtrlAlt:
						m_KeyList.push_back (keyCtrlHold);
						m_KeyList.push_back (keyAltHold);
						bPushShiftRelease = false;
						bPushAltRelease = true;
						bPushCtrlRelease = true;
						break;
					case Key_CtrlShift:
						m_KeyList.push_back (keyCtrlHold);
						m_KeyList.push_back (keyShiftHold);
						bPushShiftRelease = true;
						bPushAltRelease = false;
						bPushCtrlRelease = true;
						break;
					case Key_AltShift:
						m_KeyList.push_back (keyShiftHold);
						m_KeyList.push_back (keyAltHold);
						bPushShiftRelease = true;
						bPushAltRelease = true;
						bPushCtrlRelease = false;
						break;
					case Key_CtrlAltShift:
						m_KeyList.push_back (keyCtrlHold);
						m_KeyList.push_back (keyAltHold);
						m_KeyList.push_back (keyShiftHold);
						bPushShiftRelease = true;
						bPushAltRelease = true;
						bPushCtrlRelease = true;
						break;
					case Key_StartRepeat:
						if (c == '>')
							tempKey.nOptional = 0;
						else
						{
							CString sDest;
							nTempIdx += GetKeyParam (m_sKeys, sDest, nTempIdx);
							tempKey.nOptional = atoi (sDest);
							m_KeyList.push_back (tempKey);
						}
						bPushShiftRelease = false;
						bPushAltRelease = false;
						bPushCtrlRelease = false;
						break;
					case Key_Delay:
						if (c == '>')
							tempKey.nOptional = 0;
						else
						{
							CString sDest;
							nTempIdx += GetKeyParam (m_sKeys, sDest, nTempIdx);
							tempKey.nOptional = atoi (sDest);
							m_KeyList.push_back (tempKey);
						}
						bPushShiftRelease = false;
						bPushAltRelease = false;
						bPushCtrlRelease = false;
						break;
					}
					if (nType != Key_NotFound)
						idx = nTempIdx;
					break;
				}
				else
				{
					szTemp [nOffset] = c;
					++nTempIdx;
					++nOffset;
				}
			}
			// Not found!
			if (nTempIdx >= nKeyLength || (nTempIdx >= nTempIdx + MAX_SK_LEN - 1))
				bNotASpecialKey = true;
		}
	}
}

// Gets next special key form the given String...
CKeystrokeEngine::KeyType CKeystrokeEngine::GetNextSpecialKey(CString sKey, CKeystrokeEngine::KeyStruct &key)
{
	KeyType nType = Key_Normal;
	key.nSpecial = SK_None;
	key.nAction = Action_KeyPress;

	if (sKey == "ENTER")
		key.Vk = VK_RETURN;
	else if (sKey == "BACK")
		key.Vk = VK_BACK;
	else if (sKey == "TAB")
		key.Vk = VK_TAB;
	else if (sKey == "SHIFT")
		nType = Key_Shift;
	else if (sKey == "CTRL")
		nType = Key_Ctrl;
	else if (sKey == "ALT")
		nType = Key_Alt;
	else if (sKey == "CTRL+ALT")
		nType = Key_CtrlAlt;
	else if (sKey == "CTRL+SHIFT")
		nType = Key_CtrlShift;
	else if (sKey == "CTRL+ALT+SHIFT")
		nType = Key_CtrlAltShift;
	else if (sKey == "ALT+SHIFT")
		nType = Key_AltShift;
	else if (sKey == "CTRL_LOCK")
	{
		key.Vk = VK_CONTROL;
		key.nAction = Action_KeyDown;
		key.nSpecial = SK_CtrlLock;
	}
	else if (sKey == "CTRL_UNLOCK")
	{
		key.Vk = VK_CONTROL;
		key.nAction = Action_KeyUp;
		key.nSpecial = SK_CtrlUnlock;
	}
	else if (sKey == "ALT_LOCK")
	{
		key.Vk = VK_MENU;
		key.nAction = Action_KeyDown;
		key.nSpecial = SK_AltLock;
	}
	else if (sKey == "ALT_UNLOCK")
	{
		key.Vk = VK_MENU;
		key.nAction = Action_KeyUp;
		key.nSpecial = SK_AltUnlock;
	}
	else if (sKey == "SHIFT_LOCK")
	{
		key.Vk = VK_SHIFT;
		key.nAction = Action_KeyDown;
		key.nSpecial = SK_ShiftLock;
	}
	else if (sKey == "SHIFT_UNLOCK")
	{
		key.Vk = VK_SHIFT;
		key.nAction = Action_KeyUp;
		key.nSpecial = SK_ShiftUnlock;
	}
	else if (sKey == "PAUSE")
		key.Vk = VK_PAUSE;
	else if (sKey == "CAPSLOCK")
		key.Vk = VK_CAPITAL;
	else if (sKey == "CAPSLOCK_ON")
	{
		key.Vk = VK_CAPITAL;
		if (LOBYTE (GetKeyState (VK_CAPITAL)) != 0)
			nType = Key_Ignore;
	}
	else if (sKey == "CAPSLOCK_OFF")
	{
		key.Vk = VK_CAPITAL;
		if (LOBYTE (GetKeyState (VK_CAPITAL)) != 1)
			nType = Key_Ignore;
	}
	else if (sKey == "ESC")
		key.Vk = VK_ESCAPE;
	else if (sKey == "PAGEUP")
		key.Vk = VK_PRIOR;
	else if (sKey == "PAGEDOWN")
		key.Vk = VK_NEXT;
	else if (sKey == "END")
		key.Vk = VK_END;
	else if (sKey == "HOME")
		key.Vk = VK_HOME;
	else if (sKey == "LEFT")
		key.Vk = VK_LEFT;
	else if (sKey == "UP")
		key.Vk = VK_UP;
	else if (sKey == "RIGHT")
		key.Vk = VK_RIGHT;
	else if (sKey == "DOWN")
		key.Vk = VK_DOWN;
	else if (sKey == "PRINTSCRN")
		key.Vk = VK_SNAPSHOT;
	else if (sKey == "INSERT")
		key.Vk = VK_INSERT;
	else if (sKey == "DELETE")
		key.Vk = VK_DELETE;
	else if (sKey == "LWIN")
		key.Vk = VK_LWIN;
	else if (sKey == "RWIN")
		key.Vk = VK_RWIN;
	else if (sKey == "APPS")
		key.Vk = VK_APPS;
	else if (sKey == "NUMPAD0")
		key.Vk = VK_NUMPAD0;
	else if (sKey == "NUMPAD1")
		key.Vk = VK_NUMPAD1;
	else if (sKey == "NUMPAD2")
		key.Vk = VK_NUMPAD2;
	else if (sKey == "NUMPAD3")
		key.Vk = VK_NUMPAD3;
	else if (sKey == "NUMPAD4")
		key.Vk = VK_NUMPAD4;
	else if (sKey == "NUMPAD5")
		key.Vk = VK_NUMPAD5;
	else if (sKey == "NUMPAD6")
		key.Vk = VK_NUMPAD6;
	else if (sKey == "NUMPAD7")
		key.Vk = VK_NUMPAD7;
	else if (sKey == "NUMPAD8")
		key.Vk = VK_NUMPAD8;
	else if (sKey == "NUMPAD9")
		key.Vk = VK_NUMPAD9;
	else if (sKey == "F1")
		key.Vk = VK_F1;
	else if (sKey == "F2")
		key.Vk = VK_F2;
	else if (sKey == "F3")
		key.Vk = VK_F3;
	else if (sKey == "F4")
		key.Vk = VK_F4;
	else if (sKey == "F5")
		key.Vk = VK_F5;
	else if (sKey == "F6")
		key.Vk = VK_F6;
	else if (sKey == "F7")
		key.Vk = VK_F7;
	else if (sKey == "F8")
		key.Vk = VK_F8;
	else if (sKey == "F9")
		key.Vk = VK_F9;
	else if (sKey == "F10")
		key.Vk = VK_F10;
	else if (sKey == "F11")
		key.Vk = VK_F11;
	else if (sKey == "F12")
		key.Vk = VK_F12;
	else if (sKey == "NUMLOCK")
		key.Vk = VK_NUMLOCK;
	else if (sKey == "NUMLOCK_ON")
	{
		key.Vk = VK_NUMLOCK;
		if (LOBYTE (GetKeyState (VK_NUMLOCK)) != 0)
			nType = Key_Ignore;
	}
	else if (sKey == "NUMLOCK_OFF")
	{
		key.Vk = VK_NUMLOCK;
		if (LOBYTE (GetKeyState (VK_NUMLOCK)) != 1)
			nType = Key_Ignore;
	}
	else if (sKey == "SCROLLLOCK")
		key.Vk = VK_SCROLL;
	else if (sKey == "SCROLLLOCK_ON")
	{
		key.Vk = VK_SCROLL;
		if (LOBYTE (GetKeyState (VK_SCROLL)) != 0)
			nType = Key_Ignore;
	}
	else if (sKey == "SCROLLLOCK_OFF")
	{
		key.Vk = VK_SCROLL;
		if (LOBYTE (GetKeyState (VK_SCROLL)) != 1)
			nType = Key_Ignore;
	}
	else if (sKey == "REPEAT")
	{
		key.nSpecial = SK_StartRepeat;
		nType = Key_StartRepeat;
	}
	else if (sKey == "END_REPEAT")
	{
		key.nSpecial = SK_EndRepeat;
	}
	else if (sKey == "DELAY")
	{
		key.nSpecial = SK_Delay;
		nType = Key_Delay;
	}
	else
		return Key_NotFound;
	
	key.nScan = MapVirtualKey (key.Vk, 0);

	return nType;
}

// xxxxxxx> þeklindeki bir stringde xxxxxx i return eder. eðer sonu > ile
// bitmiyorsa false return eder
int CKeystrokeEngine::GetKeyParam (const CString &sSource, CString &sDest, int nStart)
{
	sDest.Empty ();
	int idx = 1;
	for (int i = nStart; i < sSource.GetLength (); i++)
	{
		if (sSource.GetAt (i) == '>')
			return idx;
		sDest += sSource.GetAt (i);
		++ idx;
	}
	return 0;
}

void CKeystrokeEngine::SetSendToWnd(bool bSendToWnd)
{
	m_bSendToWnd = bSendToWnd;
}

void CKeystrokeEngine::SetPause(bool bPause, int nPause)
{
	m_bPause = bPause;
	if (nPause > 0)
		m_nPause = nPause;
	else
		bPause = false;
}

// if lpszTitle == NULL, then the feature is disabled
void CKeystrokeEngine::SetWndTitle(LPCSTR lpszTitle)
{
	m_lpszWndTitle = lpszTitle;
}

// if lpszClass == NULL, then the feature is disabled
void CKeystrokeEngine::SetWndClass(LPCSTR lpszClass)
{
	m_lpszWndClass = lpszClass;
}

void CKeystrokeEngine::SetExactMatch(bool bExact)
{
	m_bExact = bExact;
}

void CKeystrokeEngine::SetCaseSensitive(bool bCaseSensitive)
{
	m_bCaseSensitive = bCaseSensitive;
}

void CKeystrokeEngine::SetReActivate(bool bReactivate, int nMiliseconds)
{
	m_bReactivate = bReactivate;
	if (nMiliseconds >= 0)
		m_nReActivate = nMiliseconds;
	else
		m_bReactivate = false;
}