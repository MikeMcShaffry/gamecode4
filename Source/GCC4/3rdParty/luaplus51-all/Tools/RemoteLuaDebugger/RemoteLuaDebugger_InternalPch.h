// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__3A6F1D26_0CBE_4B05_BF19_1EFE5ED0A8C4__INCLUDED_)
#define AFX_STDAFX_H__3A6F1D26_0CBE_4B05_BF19_1EFE5ED0A8C4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include <atlbase.h>
#include <afxmt.h>
#include <atlcoll.h>

#include <mmsystem.h>
#include <vector>
#include <algorithm>

#include "LuaPlus/LuaPlus.h"
using namespace LuaPlus;
#include "ScintillaEditor.h"
#include "Codex\cdxCDynamicFormView.h"
#include "Codex\cdxCDynamicDialog.h"
#include "Codex\cdxCDynamicBar.h"
#include "Codex\sizecbar.h"
#include "Codex\scbarg.h"
#include "LuaRemoteDebuggingServer/LuaNetworkClient.h"
using namespace LuaDebugger;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__3A6F1D26_0CBE_4B05_BF19_1EFE5ED0A8C4__INCLUDED_)
