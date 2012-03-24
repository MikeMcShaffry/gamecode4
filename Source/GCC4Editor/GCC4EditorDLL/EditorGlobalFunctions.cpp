//=======================================================================
// EditorGlobalFunctions.cpp : exports game engine functions to C++ DLL
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
//========================================================================

#include "GCC4EditorStd.h"
#include "EditorGlobalFunctions.h"

//#define INVALID_ID -1;

#include "..\..\GCC4\Actors\ActorFactory.h"
#include "..\..\GCC4\Actors\ActorComponent.h"
#include "..\..\GCC4\Graphics3D\Raycast.h"


//
// EditorMain								- Chapter 20, page 753
//
// This function is similar to the Main in GameCode4, but with a few
// key differences. We don't want to use GameCode's main loop since
// that would lock out the C# app.
//

int EditorMain(int *instancePtrAddress, int *hPrevInstancePtrAddress, int *hWndPtrAddress, int nCmdShow, int screenWidth, int screenHeight)
{
   // C# passes HINSTANCE and HWND values to C++ DLL as (int *)
	HINSTANCE hInstance = (HINSTANCE)instancePtrAddress;
	HINSTANCE hPrevInstance = (HINSTANCE) hPrevInstancePtrAddress;
	HWND hWnd = (HWND)hWndPtrAddress;
	WCHAR *lpCmdLine = L"";

	// Set up checks for memory leaks.
	// Game Coding Complete reference - Chapter 12, page XXX
	//
	int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

	// set this flag to keep memory blocks around
	tmpDbgFlag |= _CRTDBG_DELAY_FREE_MEM_DF;				// this flag will cause intermittent pauses in your game!

	// perform memory check for each alloc/dealloc
	//tmpDbgFlag |= _CRTDBG_CHECK_ALWAYS_DF;				// remember this is VERY VERY SLOW!

	// always perform a leak check just before app exits.
	tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;					

	_CrtSetDbgFlag(tmpDbgFlag);

    // [rez] Initialize the logging system as the very first thing you ever do!   LOL after the memory system flags are set, that is!
    Logger::Init("logging.xml");

	g_pApp->m_Options.Init("EditorOptions.xml", lpCmdLine);

    // Set the callback functions. These functions allow the sample framework to notify
    // the application about device changes, user input, and windows messages.  The 
    // callbacks are optional so you need only set callbacks for events you're interested 
    // in. However, if you don't handle the device reset/lost callbacks then the sample 
    // framework won't be able to reset your device since the application must first 
    // release all device resources before resetting.  Likewise, if you don't handle the 
    // device created/destroyed callbacks then the sample framework won't be able to 
    // recreate your device resources.

	DXUTSetCallbackMsgProc( GameCodeApp::MsgProc );
	DXUTSetCallbackFrameMove( GameCodeApp::OnUpdateGame );
    DXUTSetCallbackDeviceChanging( GameCodeApp::ModifyDeviceSettings );

	if (g_pApp->m_Options.m_Renderer == "Direct3D 9")
	{
		DXUTSetCallbackD3D9DeviceAcceptable( GameCodeApp::IsD3D9DeviceAcceptable );
		DXUTSetCallbackD3D9DeviceCreated( GameCodeApp::OnD3D9CreateDevice );
		DXUTSetCallbackD3D9DeviceReset( GameCodeApp::OnD3D9ResetDevice );
		DXUTSetCallbackD3D9DeviceLost( GameCodeApp::OnD3D9LostDevice );
		DXUTSetCallbackD3D9DeviceDestroyed( GameCodeApp::OnD3D9DestroyDevice );
		DXUTSetCallbackD3D9FrameRender( GameCodeApp::OnD3D9FrameRender );
	}
	else if (g_pApp->m_Options.m_Renderer == "Direct3D 11")
	{
		DXUTSetCallbackD3D11DeviceAcceptable( GameCodeApp::IsD3D11DeviceAcceptable );
		DXUTSetCallbackD3D11DeviceCreated( GameCodeApp::OnD3D11CreateDevice );
		DXUTSetCallbackD3D11SwapChainResized( GameCodeApp::OnD3D11ResizedSwapChain );
		DXUTSetCallbackD3D11SwapChainReleasing( GameCodeApp::OnD3D11ReleasingSwapChain );
		DXUTSetCallbackD3D11DeviceDestroyed( GameCodeApp::OnD3D11DestroyDevice );
		DXUTSetCallbackD3D11FrameRender( GameCodeApp::OnD3D11FrameRender );	
	}
	else
	{
		GCC_ASSERT(0 && "Unknown renderer specified in game options.");
		return false;
	}

    // Show the cursor and clip it when in full screen
    DXUTSetCursorSettings( true, true );

	// Perform application initialization
	if (!g_pApp->InitInstance (hInstance, lpCmdLine, hWnd, screenWidth, screenHeight)) 
	{
		return FALSE;
	}

   // This is where the game would normally call the main loop, but the
   // C# application will control this, so we don’t need to call 
   // DXUTMainLoop() here.

	return true;
}


//
// RenderFrame							- Chapter 22, page 755
//
void RenderFrame()
{
	// In TeapotWars, this would be called by GameCode's main loop
	// Since the C# app has its own main loop, we expose this
	// function so that C# app can call from its main loop
	DXUTRender3DEnvironment(); 
}

//
// Shutdown								- Chapter 22, page 755
//
int Shutdown()
{
	// Normally this is called after the GameCode main loop ends. We
	// expose this here so that the C# app can shutdown after it finishes
	// its main loop
	DXUTShutdown();

    return g_pApp->GetExitCode();	
}

//
// WndProc								- Chapter 22, page 755
//
void WndProc(int *hWndPtrAddress, int msg, int wParam, int lParam)
{
	HWND hWnd = (HWND)hWndPtrAddress;
	DXUTStaticWndProc( hWnd, msg, WPARAM(wParam), LPARAM(lParam) );
}



//
// IsGameRunning						- Chapter 22   
//
int IsGameRunning()
{
	EditorLogic* game = (EditorLogic*)g_pApp->m_pGame;
   return (game) ? game->IsRunning() : false;
}



//
// OpenLevel							- Chapter 22, page 756
//
//   FUTURE WORK: This should return a bool specifying if the level was successfully opened.
//
void OpenLevel( BSTR fullPathLevelFile )
{
	// We need to strip off the project directory from the filename first.
	std::string levelFile = ws2s(std::wstring(fullPathLevelFile, SysStringLen(fullPathLevelFile))); 
	EditorLogic* pEditorLogic = (EditorLogic*)g_pApp->m_pGame;
	if (pEditorLogic)
	{
		std::string assetsDir = "\\Assets\\";
		int projDirLength = pEditorLogic->GetProjectDirectory().length() + assetsDir.length();
		g_pApp->m_Options.m_Level = levelFile.substr(projDirLength, levelFile.length()-projDirLength);
		pEditorLogic->VChangeState(BGS_LoadingGameEnvironment);
	}
}




//
// GetNumActors							- Chapter 22, page 757
//
int GetNumActors()
{
	EditorLogic* pGame = (EditorLogic*)g_pApp->m_pGame;
	return (pGame) ? pGame->GetNumActors() : 0;
}


//
// GetActorList							- Chapter 22, page 757
//
void GetActorList( int *ptr, int numActors )
{
	// To keep things simple, we pass the actor ids to the C# app
	// the C# app iterates through the actor ids, and calls back into
	// the unmanaged dll to get the appropriate information about each
	// actor
	EditorLogic* pGame = (EditorLogic*)g_pApp->m_pGame;
	if ( pGame )
	{
		ActorMap::const_iterator itr;
		int actorArrayIndex;
		for ( itr = pGame->GetActorMap().begin(), actorArrayIndex = 0; 
			itr != pGame->GetActorMap().end() && actorArrayIndex < numActors; itr++, actorArrayIndex++ )
		{
			ActorId actorId = itr->first;
			ptr[actorArrayIndex] = actorId;
		}
	}
}


//
// GetActorXmlSize						- Chapter 22, page 757
//
int GetActorXmlSize ( ActorId actorId )
{
	StrongActorPtr pActor = MakeStrongPtr(g_pApp->m_pGame->VGetActor(actorId));
	if ( !pActor )
	{
		return 0;
	}
	std::string xml = pActor->ToXML();
	return xml.length();
}


//
// GetActorXml							- Chapter 22, page 758
//
void GetActorXml ( int *actorXMLAddress, ActorId actorId )
{
	StrongActorPtr pActor = MakeStrongPtr(g_pApp->m_pGame->VGetActor(actorId));
	if ( !pActor )
	{
		return;
	}
	std::string xml = pActor->ToXML();
	
	strncpy_s(reinterpret_cast<char *>(actorXMLAddress), xml.length()+1, xml.c_str(), xml.length());
}



//
// PickActor									- Chapter 22, page 760
//
int PickActor(int *hWndPtrAddress)
{
	HWND hWnd = (HWND)hWndPtrAddress;

	POINT ptCursor;
	GetCursorPos( &ptCursor );
	
	// Convert the screen coordinates of the mouse cursor into
	// coordinates relative to the client window
	ScreenToClient( hWnd, &ptCursor );
	RayCast rayCast(ptCursor);
	EditorLogic* pGame = (EditorLogic*)g_pApp->m_pGame;

	if (!pGame)
	{
		return INVALID_ACTOR_ID;
	}

	shared_ptr<EditorHumanView> pView = pGame->GetHumanView();
	if (!pView)
	{
		return INVALID_ACTOR_ID;
	}


	// Cast a ray through the scene. The RayCast object contains an array of Intersection
	// objects.
	pView->GetScene()->Pick(&rayCast);
	rayCast.Sort();
	
	// If there are any intersections, get information from the first intersection.
   if (!rayCast.m_NumIntersections)
   {
      return INVALID_ACTOR_ID;
   }

   Intersection firstIntersection = rayCast.m_IntersectionArray[0];
   return firstIntersection.m_actorId;
}

//
// CreateActor							- Chapter 22, page 761
//
int CreateActor( BSTR bstrActorXMLFile )
{
	std::string actorResource = ws2s(std::wstring(bstrActorXMLFile, SysStringLen(bstrActorXMLFile))); 
	StrongActorPtr pActor = g_pApp->m_pGame->VCreateActor(actorResource, NULL);
	if (!pActor)
		return INVALID_ACTOR_ID;

	// fire an event letting everyone else know that we created a new actor
	shared_ptr<EvtData_New_Actor> pNewActorEvent(GCC_NEW EvtData_New_Actor(pActor->GetId()));
	IEventManager::Get()->VQueueEvent(pNewActorEvent);
	return pActor->GetId();
}

//
// ModifyActor							- Chapter 22, page 762
//
void ModifyActor ( BSTR bstrActorModificationXML )
{
	std::string actorModificationXML = ws2s(std::wstring(bstrActorModificationXML, SysStringLen(bstrActorModificationXML))); 

	TiXmlDocument doc;
	doc.Parse(actorModificationXML.c_str());
    TiXmlElement* pRoot = doc.RootElement();
    if (!pRoot)
        return;

	g_pApp->m_pGame->VModifyActor(atoi(pRoot->Attribute("id")), pRoot);
}

//
// DestroyActor							- Chapter 22, page 763
//
void DestroyActor( ActorId actorId )
{
	g_pApp->m_pGame->VDestroyActor(actorId);
}







//
// GetActorXmlSize						- Chapter 22, page 757
//
int GetLevelScriptAdditionsXmlSize ( )
{
	TiXmlDocument *optionsDoc = g_pApp->m_Options.m_pDoc;
	TiXmlElement *pRoot = optionsDoc->RootElement();
    if (!pRoot)
		return 0;

	TiXmlElement *worldScriptAdditions = pRoot->FirstChildElement("WorldScriptAdditions");

	TiXmlDocument outDoc;
	TiXmlNode *node = worldScriptAdditions->Clone();
    outDoc.LinkEndChild(node);

	TiXmlPrinter printer;
	outDoc.Accept(&printer);

	std::string xml = printer.CStr();

	return xml.length();
}


//
// GetActorXml							- Chapter 22, page 758
//
void GetLevelScriptAdditionsXml ( int *xmlAddress )
{
	TiXmlDocument *optionsDoc = g_pApp->m_Options.m_pDoc;
	TiXmlElement *pRoot = optionsDoc->RootElement();
    if (!pRoot)
		return;

	TiXmlElement *worldScriptAdditions = pRoot->FirstChildElement("WorldScriptAdditions");

	TiXmlDocument outDoc;
	TiXmlNode *node = worldScriptAdditions->Clone();
    outDoc.LinkEndChild(node);


	TiXmlPrinter printer;
	outDoc.Accept(&printer);

	std::string xml = printer.CStr();

	strncpy_s(reinterpret_cast<char *>(xmlAddress), xml.length()+1, xml.c_str(), xml.length());
}

