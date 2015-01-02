#pragma once
//========================================================================
// GameCodeStd.h : include file for standard system include files,
//            or project specific include files that are used frequently, but
//            are changed infrequently
//
// GameCode4 is the sample application that encapsulates much of the source code
// discussed in "Game Coding Complete - 4th Edition" by Mike McShaffry and David
// "Rez" Graham, published by Charles River Media. 
// ISBN-10: 1133776574 | ISBN-13: 978-1133776574
//
// If this source code has found it's way to you, and you think it has helped you
// in any way, do the authors a favor and buy a new copy of the book - there are 
// detailed explanations in it that compliment this code well. Buy a copy at Amazon.com
// by clicking here: 
//    http://www.amazon.com/gp/product/1133776574/ref=olp_product_details?ie=UTF8&me=&seller=
//
// There's a companion web site at http://www.mcshaffry.com/GameCode/
// 
// The source code is managed and maintained through Google Code: 
//    http://code.google.com/p/gamecode4/
//
// (c) Copyright 2012 Michael L. McShaffry and David Graham
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser GPL v3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See 
// http://www.gnu.org/licenses/lgpl-3.0.txt for more details.
//
// You should have received a copy of the GNU Lesser GPL v3
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//========================================================================

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// Windows Header Files:

#define NOMINMAX
#include <windows.h>
#include <windowsx.h>

#include <crtdbg.h>

#define _VS2005_
#if _MSC_VER < 1400
	#undef _VS2005_
#endif

// [rez] I am saddened that Visual Studio 2010 doesn't support the new C++0x final keyword.  It does support 
// the override keyword as well as a special Microsoft-specific sealed keyword.  The override keyword works 
// exactly as the C++0x spec says it does and the sealed keyword acts like the C++0x final keyword.  I toyed 
// around with whether or not I wanted to include this so I will leave the decision up to you.  If you want 
// to use the new C++0x style of override & final, just uncomment this line.  This works on Visual Studio 2008 
// and up for sure.  I'm not sure about earlier versions.
//#define final sealed


// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <mmsystem.h>

#include <algorithm>
#include <string>
#include <list>
#include <vector>
#include <queue>
#include <map>

// [rez] Smart pointers were added to the std namespace in Visual Studio 2012.  They exist in 2010 in the 
// std::tr1 namespace.
#if _MSC_VER >= 1700  // VS 2012 or later
    #include <memory>
    using std::shared_ptr;
    using std::weak_ptr;
    using std::static_pointer_cast;
    using std::dynamic_pointer_cast;
#elif _MSC_VER == 1600  // VS 2010
    using std::tr1::shared_ptr;
    using std::tr1::weak_ptr;
    using std::tr1::static_pointer_cast;
    using std::tr1::dynamic_pointer_cast;
#endif

class GCC_noncopyable 
{  
     private:  
      GCC_noncopyable(const GCC_noncopyable& x);  
      GCC_noncopyable& operator=(const GCC_noncopyable& x);  
     public:  
      GCC_noncopyable() {}; // Default constructor  
};  


// Game Code Complete - Chapter 12, page 446-447
#if defined(_DEBUG)
#	define GCC_NEW new(_NORMAL_BLOCK,__FILE__, __LINE__)
#else
#	define GCC_NEW new
#endif

#define DXUT_AUTOLIB

// DirectX Includes
#include <dxut.h>
#include <d3dx9tex.h>
#include <dxut.h>
#include <SDKmisc.h>

#include <tinyxml.h>

// fast delegate stuff
#include "../3rdParty/FastDelegate/FastDelegate.h"
using fastdelegate::MakeDelegate;

#pragma warning( disable : 4996 ) // 'function' declared deprecated - gets rid of all those 2005 warnings....


// GameCode #includes
#include "..\Debugging\Logger.h"  // this should be the first of the gcc includes since it defines GCC_ASSERT()
#include "..\Utilities\types.h"
#include "..\Utilities\templates.h"
#include "..\Graphics3D\geometry.h"

typedef D3DXCOLOR Color;

extern Color g_White;
extern Color g_Black;
extern Color g_Cyan;
extern Color g_Red;
extern Color g_Green;
extern Color g_Blue;
extern Color g_Yellow;
extern Color g_Gray40;
extern Color g_Gray25;
extern Color g_Gray65;
extern Color g_Transparent;

extern Vec3 g_Up;
extern Vec3 g_Right;
extern Vec3 g_Forward;

extern Vec4 g_Up4;
extern Vec4 g_Right4;
extern Vec4 g_Forward4;



//  AppMsg				- Chapter 9, page 248

struct AppMsg
{
	HWND m_hWnd;
	UINT m_uMsg;
	WPARAM m_wParam;
	LPARAM m_lParam;
};



#include "..\GameCode4\interfaces.h"


// Useful #defines

extern const float fOPAQUE;
extern const int iOPAQUE;
extern const float fTRANSPARENT;
extern const int iTRANSPARENT;

extern const int MEGABYTE;
extern const float SIXTY_HERTZ;

extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;

#if !defined(SAFE_DELETE)
	#define SAFE_DELETE(x) if(x) delete x; x=NULL;
#endif

#if !defined(SAFE_DELETE_ARRAY)
	#define SAFE_DELETE_ARRAY(x) if (x) delete [] x; x=NULL; 
#endif

#if !defined(SAFE_RELEASE)
	#define SAFE_RELEASE(x) if(x) x->Release(); x=NULL;
#endif

#ifdef UNICODE
	#define _tcssprintf wsprintf
	#define tcsplitpath _wsplitpath
#else
	#define _tcssprintf sprintf
	#define tcsplitpath _splitpath
#endif

#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __LOC__ __FILE__ "("__STR1__(__LINE__)") : Warning Msg: "

#include "..\GameCode4\GameCode.h"

extern INT WINAPI GameCode4(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPWSTR    lpCmdLine,
                     int       nCmdShow);

