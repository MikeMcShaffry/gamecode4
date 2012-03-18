#pragma once
//========================================================================
// EditorGlobalFunctions.h : exports game engine functions to C++ DLL
//
// Part of the GameCode4 Application
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

#define DllExport _declspec(dllexport)

#include "Editor.h"

//========================================================================
//
// This file contains functions that will be exposed via the dll wrapper
// to the C# app. The functions will be used by the C# app to query information
// about the editor, and to change the editor's scene.
//
//		- Chapter 22, page 763
//
//========================================================================


// Editor Framework Functions
extern "C" DllExport int EditorMain(int *instancePtrAddress, int *hPrevInstancePtrAddress, int *hWndPtrAddress, int nCmdShow, int screenWidth, int screenHeight);
extern "C" DllExport void WndProc(int *hWndPtrAddress, int msg, int wParam, int lParam);
extern "C" DllExport int IsGameRunning();
extern "C" DllExport void RenderFrame();
extern "C" DllExport int Shutdown();
extern "C" DllExport void OpenLevel( BSTR lFileName );

// Actor accessor functions
extern "C" DllExport int GetNumActors();
extern "C" DllExport void GetActorList( int *actorIdArrayPtrAddress, int size );
extern "C" DllExport int GetActorXmlSize ( ActorId actorId );
extern "C" DllExport void GetActorXml ( int *actorXmlPtrAddress, ActorId actorId );
extern "C" DllExport int PickActor(int *hWndPtrAddress);

// Actor Modification functions
extern "C" DllExport int CreateActor( BSTR bstrActorResource );
extern "C" DllExport void ModifyActor ( BSTR bstrActorModificationXML );
extern "C" DllExport void DestroyActor( ActorId actorId );

// Level script additions - added post press
extern "C" DllExport int GetLevelScriptAdditionsXmlSize (  );
extern "C" DllExport void GetLevelScriptAdditionsXml ( int *xmlPtrAddress );

