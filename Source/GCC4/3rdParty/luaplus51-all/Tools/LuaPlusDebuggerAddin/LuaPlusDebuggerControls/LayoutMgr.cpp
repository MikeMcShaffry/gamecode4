// --------------------------------------------------------------------------
// Description : several WTL utility classes for managing the layout of window children
// Author	   : Serge Weinstock
//
//	You are free to use, distribute or modify this code
//	as long as this header is not removed or modified.
// --------------------------------------------------------------------------
#include "stdafx.h"
#include "LayoutMgr.h"

//=============================================================================
// Private messages
//=============================================================================
const RegisteredWindowMessage	WMU_PS_ATTACH_PAGE(_T("WTLU_LM::WMU_PS_ATTACH_PAGE"));
const RegisteredWindowMessage	WMU_PS_INIT(_T("WTLU_LM::WMU_PS_INIT"));
const RegisteredWindowMessage	WMU_PS_ATTACH_PS(_T("WTLU_LM::WMU_PS_ATTACH_PS"));

//=============================================================================
// 
//=============================================================================
int SysInfo::_isThemeSupported = -1;
