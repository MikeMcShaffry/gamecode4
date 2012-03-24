//========================================================================
// GameCode.cpp : Defines the Application layer for GameCode4
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

#include "GameCodeStd.h"

#include "Dbt.h"						// required for DBT_DEVICEREMOVECOMPLETE

#include "../MainLoop/Initialization.h"
#include "../Debugging/MiniDump.h"
#include "../GameCode4/BaseGameLogic.h"
#include "../Graphics3D/D3DRenderer.h"
#include "../EventManager/EventManagerImpl.h"
#include "../Network/Network.h"
#include "../LUAScripting/LuaStateManager.h"
#include "../LUAScripting/ScriptExports.h"
#include "../LUAScripting/ScriptProcess.h"
#include "../ResourceCache/ResCache.h"
#include "../ResourceCache/XmlResource.h"
#include "../UserInterface/UserInterface.h"
#include "../UserInterface/MessageBox.h"
#include "../UserInterface/HumanView.h"
#include "../Utilities/Math.h"
#include "../Utilities/String.h"
#include "../Actors/BaseScriptComponent.h"

// All event type headers
#include "../Physics/PhysicsEventListener.h"
#include "../EventManager/Events.h"

#define MAX_LOADSTRING 100


GameCodeApp *g_pApp = NULL;

const char* SCRIPT_PREINIT_FILE = "Scripts\\PreInit.lua";


//========================================================================
//
// GameCodeApp implementation
//
//========================================================================

GameCodeApp::GameCodeApp()
{
	g_pApp = this;
	m_pGame = NULL;

	m_rcDesktop.bottom = m_rcDesktop.left = m_rcDesktop.right = m_rcDesktop.top = 0;
	m_screenSize = Point(0,0);
	m_iColorDepth = 32;

	m_bIsRunning = false;
	m_bIsEditorRunning = false;

	m_pEventManager = NULL;
	m_ResCache = NULL;

	m_pNetworkEventForwarder = NULL; 
	m_pBaseSocketManager = NULL;

	m_bQuitRequested = false;
	m_bQuitting = false;
	m_HasModalDialog = 0;
}


HWND GameCodeApp::GetHwnd()
{ 
	return DXUTGetHWND();
}


//===================================================================
// Win32 Specific Stuff
//
// InitInstance - this checks system resources, creates your window, and launches the game
//
// preprocessor macro setting in the C++ options of the project provides the base macro
// C preprocessor string concatenation takes care of the rest.
//
// GameCodeApp::InitInstance - Chapter 5, page 136
//
//===================================================================

bool GameCodeApp::InitInstance(HINSTANCE hInstance, LPWSTR lpCmdLine, HWND hWnd, int screenWidth, int screenHeight)
{
	// Check for existing instance of the same window
	// 
#ifndef _DEBUG
	// Note - it can be really useful to debug network code to have
	// more than one instance of the game up at one time - so
	// feel free to comment these lines in or out as you wish!
	if (!IsOnlyInstance(VGetGameTitle()))
		return false;
#endif

	// We don't need a mouse cursor by default, let the game turn it on
	SetCursor( NULL );

	// Check for adequate machine resources.
	bool resourceCheck = false;
	while (!resourceCheck)
	{
		const DWORDLONG physicalRAM = 512 * MEGABYTE;
		const DWORDLONG virtualRAM = 1024 * MEGABYTE;
		const DWORDLONG diskSpace = 10 * MEGABYTE;
		if (!CheckStorage(diskSpace))
			return false;

		const DWORD minCpuSpeed = 1300;			// 1.3Ghz
		DWORD thisCPU = ReadCPUSpeed();
		if (thisCPU < minCpuSpeed)
		{
			GCC_ERROR("GetCPUSpeed reports CPU is too slow for this game.");
			return false;
		}

		resourceCheck = true;
	}

	m_hInstance = hInstance;

    // register all events
    RegisterEngineEvents();
    VRegisterGameEvents();

	//
	// Initialize the ResCache - Chapter 5, page 141
	//
	//    Note - this is a little different from the book. Here we have a speccial resource ZIP file class, DevelopmentResourceZipFile,
	//    that actually reads directly from the source asset files, rather than the ZIP file. This is MUCH better during development, since
	//    you don't want to have to rebuild the ZIP file every time you make a minor change to an asset.
	//
	IResourceFile *zipFile = (m_bIsEditorRunning || m_Options.m_useDevelopmentDirectories) ? 
		GCC_NEW DevelopmentResourceZipFile(L"Assets.zip", DevelopmentResourceZipFile::Editor) :
		GCC_NEW ResourceZipFile(L"Assets.zip");

	m_ResCache = GCC_NEW ResCache(50, zipFile);

	if (!m_ResCache->Init())
	{
        GCC_ERROR("Failed to initialize resource cache!  Are your paths set up correctly?");
		return false;
	}
	
	extern shared_ptr<IResourceLoader> CreateWAVResourceLoader();
	extern shared_ptr<IResourceLoader> CreateOGGResourceLoader();
	extern shared_ptr<IResourceLoader> CreateDDSResourceLoader();
	extern shared_ptr<IResourceLoader> CreateJPGResourceLoader();
    extern shared_ptr<IResourceLoader> CreateXmlResourceLoader();
    extern shared_ptr<IResourceLoader> CreateSdkMeshResourceLoader();
    extern shared_ptr<IResourceLoader> CreateScriptResourceLoader();

	// Note - register these in order from least specific to most specific! They get pushed onto a list.
	// RegisterLoader is discussed in Chapter 5, page 142
	m_ResCache->RegisterLoader(CreateWAVResourceLoader());
	m_ResCache->RegisterLoader(CreateOGGResourceLoader());
	m_ResCache->RegisterLoader(CreateDDSResourceLoader());
	m_ResCache->RegisterLoader(CreateJPGResourceLoader());
    m_ResCache->RegisterLoader(CreateXmlResourceLoader());
	m_ResCache->RegisterLoader(CreateSdkMeshResourceLoader());
    m_ResCache->RegisterLoader(CreateScriptResourceLoader());

	if(!LoadStrings("English"))
	{
        GCC_ERROR("Failed to load strings");
		return false;
	}

	// Rez up the Lua State manager now, and run the initial script - discussed in Chapter 5, page 144.
    if (!LuaStateManager::Create())
	{
        GCC_ERROR("Failed to initialize Lua");
		return false;
	}

    // Load the preinit file.  This is within braces to create a scope and destroy the resource once it's loaded.  We
    // don't need to do anything with it, we just need to load it.
    {
        Resource resource(SCRIPT_PREINIT_FILE);
        shared_ptr<ResHandle> pResourceHandle = m_ResCache->GetHandle(&resource);  // this actually loads the XML file from the zip file
    }

    // Register function exported from C++
    ScriptExports::Register();
    ScriptProcess::RegisterScriptClass();
    BaseScriptComponent::RegisterScriptFunctions();

	// The event manager should be created next so that subsystems can hook in as desired.
	// Discussed in Chapter 5, page 144
	m_pEventManager = GCC_NEW EventManager("GameCodeApp Event Mgr", true );
	if (!m_pEventManager)
	{
		GCC_ERROR("Failed to create EventManager.");
		return false;
	}

	// DXUTInit, DXUTCreateWindow - Chapter 5, page 145	
	DXUTInit( true, true, lpCmdLine, true ); // Parse the command line, handle the default hotkeys, and show msgboxes

	if (hWnd==NULL)
	{
	    DXUTCreateWindow( VGetGameTitle(), hInstance, VGetIcon() );
	}
	else
	{
	    DXUTSetWindow( hWnd, hWnd, hWnd );
	}

	if (!GetHwnd())
	{
		return FALSE;
	}
	SetWindowText(GetHwnd(), VGetGameTitle());

	// initialize the directory location you can store save game files
	_tcscpy_s(m_saveGameDirectory, GetSaveGameDirectory(GetHwnd(), VGetGameAppDirectory()));

	// DXUTCreateDevice - Chapter 5 - page 139
	m_screenSize = Point(screenWidth, screenHeight);

	//If you have an older video card that only supports D3D9, comment in the next line, and make sure 
	//the renderer setting in Game\PlayerOptions.xml is set to "Direct3D 9"
	//DXUTCreateDevice( D3D_FEATURE_LEVEL_9_3, true, screenWidth, screenHeight);
	DXUTCreateDevice( D3D_FEATURE_LEVEL_10_1, true, screenWidth, screenHeight);

	if (GetRendererImpl() == Renderer_D3D9)
	{
		m_Renderer = shared_ptr<IRenderer>(GCC_NEW D3DRenderer9());
	}
	else if (GetRendererImpl() == Renderer_D3D11)
	{
		m_Renderer = shared_ptr<IRenderer>(GCC_NEW D3DRenderer11());
	}
	m_Renderer->VSetBackgroundColor(255, 20, 20, 200);
	m_Renderer->VOnRestore();


	// You usually must have an HWND to initialize your game views...
	//    VCreateGameAndView			- Chapter 5, page 145
	m_pGame = VCreateGameAndView();
	if (!m_pGame)
		return false;

	// now that all the major systems are initialized, preload resources 
	//    Preload calls are discussed in Chapter 5, page 148
	m_ResCache->Preload("*.ogg", NULL);
	m_ResCache->Preload("*.dds", NULL);
	m_ResCache->Preload("*.jpg", NULL);

	if ( GameCodeApp::GetRendererImpl() == GameCodeApp::Renderer_D3D11 )
		m_ResCache->Preload("*.sdkmesh", NULL);

	CheckForJoystick(GetHwnd());

	m_bIsRunning = true;

	return TRUE;
}

bool GameCodeApp::VLoadGame(void)
{
    // Read the game options and see what the current game
    // needs to be - all of the game graphics are initialized by now, too...
	return m_pGame->VLoadGame(m_Options.m_Level.c_str());
}

void GameCodeApp::RegisterEngineEvents(void)
{
    REGISTER_EVENT(EvtData_Environment_Loaded);
    REGISTER_EVENT(EvtData_New_Actor);
    REGISTER_EVENT(EvtData_Move_Actor);
    REGISTER_EVENT(EvtData_Destroy_Actor);
	REGISTER_EVENT(EvtData_Request_New_Actor);
	REGISTER_EVENT(EvtData_Network_Player_Actor_Assignment);
}

//
// GameCodeApp::LoadStrings										- Chapter 5, page 143
//
bool GameCodeApp::LoadStrings(std::string language)
{
	std::string languageFile = "Strings\\";
	languageFile += language;
	languageFile += ".xml";

	TiXmlElement* pRoot = XmlResourceLoader::LoadAndReturnRootXmlElement(languageFile.c_str());
	if (!pRoot)
	{
		GCC_ERROR("Strings are missing.");
		return false;
	}

    // Loop through each child element and load the component
    for (TiXmlElement* pElem = pRoot->FirstChildElement(); pElem; pElem = pElem->NextSiblingElement())
    {
		const char *pKey=pElem->Attribute("id");
		const char *pText=pElem->Attribute("value");
		const char *pHotkey=pElem->Attribute("hotkey");
		if (pKey && pText) 
		{
			wchar_t wideKey[64];
			wchar_t wideText[1024];
			AnsiToWideCch(wideKey, pKey, 64);
			AnsiToWideCch(wideText, pText, 1024);
			m_textResource[std::wstring(wideKey)] = std::wstring(wideText);

			if (pHotkey)
			{
				m_hotkeys[std::wstring(wideKey)] = MapCharToKeycode(*pHotkey);
			}
		}
	}
	return true;
}

UINT GameCodeApp::MapCharToKeycode(const char pHotKey)
{
	if (pHotKey >= '0' && pHotKey <= '9')
		return 0x30 + pHotKey - '0';

	if (pHotKey >= 'A' && pHotKey <= 'Z')
		return 0x41 + pHotKey - 'A';

	GCC_ASSERT(0 && "Platform specific hotkey is not defined");
	return 0;	
}


//----------------------------------------------------------
// GameCodeApp::GetString								- Chapter 5, page 144
//
// creates a string from a string resource ID in the string table
// stored in a special DLL, LANG.DLL, so game text strings
// can be language independant
//
std::wstring GameCodeApp::GetString(std::wstring sID)
{
	auto localizedString = m_textResource.find(sID);
	if(localizedString == m_textResource.end())
	{
		GCC_ASSERT(0 && "String not found!");
		return L"";
	}
	return localizedString->second;
}


//----------------------------------------------------------
// Win32 Specific Message Handlers
//
// WndProc - the main message handler for the window class
//
// OnNcCreate - this is where you can set window data before it is created
// OnMove - called whenever the window moves; used to update members of g_App
// OnDeviceChange - called whenever you eject the CD-ROM.
// OnDisplayChange - called whenever the user changes the desktop settings
// OnPowerBroadcast - called whenever a power message forces a shutdown
// OnActivate - called whenever windows on the desktop change focus.
//
// Note: pUserContext added to comply with DirectX 9c - June 2005 Update
//
LRESULT CALLBACK GameCodeApp::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext )
{
    // Always allow dialog resource manager calls to handle global messages
    // so GUI state is updated correctly
    *pbNoFurtherProcessing = D3DRenderer::g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

	LRESULT result = 0;

	switch (uMsg) 
	{
		case WM_POWERBROADCAST:
		{
			int event = (int)wParam;
			result = g_pApp->OnPowerBroadcast(event);
			break;
		}

		case WM_DISPLAYCHANGE:
		{
			int colorDepth = (int)wParam;
			int width = (int)(short) LOWORD(lParam);
			int height = (int)(short) HIWORD(lParam);

			result = g_pApp->OnDisplayChange(colorDepth, width, height);
			break;
		}

		case WM_SYSCOMMAND: 
		{
			result = g_pApp->OnSysCommand(wParam, lParam);
			if (result)
			{
				*pbNoFurtherProcessing = true;
			}
			break;
		}

		case WM_SYSKEYDOWN:
		{
			if (wParam == VK_RETURN)
			{
				*pbNoFurtherProcessing = true;
				return g_pApp->OnAltEnter();
			}
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}


		case WM_CLOSE:
		{
			// DXUT apps choose ESC key as a default exit command.
			// GameCode4 doesn't like this so we disable it by checking 
			// the m_bQuitting bool, and if we're not really quitting
			// set the "no further processing" parameter to true.
			if (g_pApp->m_bQuitting)
			{
				result = g_pApp->OnClose();
			}
			else
			{
				*pbNoFurtherProcessing = true;
			}
			break;
		}


		case WM_KEYDOWN:
        case WM_KEYUP:
		case WM_CHAR:
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case MM_JOY1BUTTONDOWN:
		case MM_JOY1BUTTONUP:
		case MM_JOY1MOVE:
		case MM_JOY1ZMOVE:
		case MM_JOY2BUTTONDOWN:
		case MM_JOY2BUTTONUP:
		case MM_JOY2MOVE:
		case MM_JOY2ZMOVE:
		{
			//
			// See Chapter 10, page 278 for more explanation of this code.
			//
			if (g_pApp->m_pGame)
			{
				BaseGameLogic *pGame = g_pApp->m_pGame;
				// Note the reverse order! User input is grabbed first from the view that is on top, 
				// which is the last one in the list.
				AppMsg msg;
				msg.m_hWnd = hWnd;
				msg.m_uMsg = uMsg;
				msg.m_wParam = wParam;
				msg.m_lParam = lParam;
				for(GameViewList::reverse_iterator i=pGame->m_gameViews.rbegin(); i!=pGame->m_gameViews.rend(); ++i)
				{
					if ( (*i)->VOnMsgProc( msg ) )
					{
						result = true;
						break;				// WARNING! This breaks out of the for loop.
					}
				}
			}
			break;
		}

		/**********************
		WARNING!!!!! You MIGHT think you need this, but if you use the DirectX
		Framework the DefWindowProc is called for you....

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);

		***********************/
	}

	return result;
}


//=========================================================
// GameCodeApp::OnNcCreate
//
// Handles the WM_NCCREATE message
//
// Not described in the book
// It sets a few members of the CREATESTRUCT, so the 
// window looks like we want it to look.
//=========================================================

LRESULT GameCodeApp::OnNcCreate(LPCREATESTRUCT cs)
{
	// If you want to override something in the CREATESTRUCT, do it here!
	// You'll usually do something like change window borders, etc.
	return true;
}

//=========================================================
// GameCodeApp::OnDisplayChange - Chapter X, page Y
//
// Handles the WM_DISPLAYCHANGE message
//
//=========================================================

LRESULT GameCodeApp::OnDisplayChange(int colorDepth, int width, int height)
{
	m_rcDesktop.left = 0;
	m_rcDesktop.top = 0; 
	m_rcDesktop.right = width;
	m_rcDesktop.bottom = height;
	m_iColorDepth = colorDepth;
	return 0;
}

//=========================================================
// GameCodeApp::OnPowerBroadcast - Chapter X, page Y
//
// Handles the WM_POWERBROADCAST message
//
//=========================================================

LRESULT GameCodeApp::OnPowerBroadcast(int event)
{
	// Don't allow the game to go into sleep mode
	if ( event == PBT_APMQUERYSUSPEND )
		return BROADCAST_QUERY_DENY;
	else if ( event == PBT_APMBATTERYLOW )
	{
		AbortGame();
	}

	return true;
}

//=========================================================
// GameCodeApp::OnSysCommand - Chapter X, page Y
//
// Handles the WM_SYSCOMMAND message
//
//=========================================================

LRESULT GameCodeApp::OnSysCommand(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
		case SC_MAXIMIZE :
		{
			// If windowed and ready...
			if ( m_bWindowedMode && IsRunning() )
			{
				// Make maximize into FULLSCREEN toggle
				OnAltEnter();
			}
		}
		return 0;

		case SC_CLOSE :
		{
			// The quit dialog confirmation would appear once for
			// every SC_CLOSE we get - which happens multiple times
			// if modal dialogs are up.  This now uses the g_QuitNoPrompt
			// flag to only prompt when receiving a SC_CLOSE that isn't
			// generated by us (identified by g_QuitNoPrompt).
			
			// If closing, prompt to close if this isn't a forced quit
			if ( lParam != g_QuitNoPrompt )
			{
				// ET - 05/21/01 - Bug #1916 - Begin
				// We were receiving multiple close dialogs
				// when closing again ALT-F4 while the close
				// confirmation dialog was up.
				// Eat if already servicing a close
				if ( m_bQuitRequested )
					return true;

				// Wait for the application to be restored
				// before going any further with the new 
				// screen.  Flash until the person selects
				// that they want to restore the game, then
				// reinit the display if fullscreen.  
				// The reinit is necessary otherwise the game
				// will switch to windowed mode.
	
				// Quit requested
				m_bQuitRequested = true;
				// Prompt
				if ( MessageBox::Ask(QUESTION_QUIT_GAME) == IDNO )
				{
					// Bail - quit aborted
					
					// Reset quit requested flag
					m_bQuitRequested = false;
					
					return true;
				}
			}
	
			m_bQuitting = true;

			// Is there a game modal dialog up?
			if ( HasModalDialog() )
			{
				// Close the modal
				// and keep posting close to the app
				ForceModalExit();

				// Reissue the close to the app
				
				// Issue the new close after handling the current one,
				// but send in g_QuitNoPrompt to differentiate it from a 
				// regular CLOSE issued by the system.
				PostMessage( GetHwnd(), WM_SYSCOMMAND, SC_CLOSE, g_QuitNoPrompt );

				m_bQuitRequested = false;
				
				// Eat the close
				return true;
			}

			// Reset the quit after any other dialogs have popped up from this close
			m_bQuitRequested = false;
		}
		return 0;

		default:
			// return non-zero of we didn't process the SYSCOMMAND message
			return DefWindowProc(GetHwnd(), WM_SYSCOMMAND, wParam, lParam);
	}

	return 0;
}

//=========================================================
// GameCodeApp::OnClose - Chapter 5, page 152
//
// Handles the WM_CLOSE message
//
//=========================================================

LRESULT GameCodeApp::OnClose()
{
	// release all the game systems in reverse order from which they were created

	SAFE_DELETE(m_pGame);

	DestroyWindow(GetHwnd());

	VDestroyNetworkEventForwarder();

	SAFE_DELETE(m_pBaseSocketManager);

	SAFE_DELETE(m_pEventManager);

    BaseScriptComponent::UnregisterScriptFunctions();
    ScriptExports::Unregister();
    LuaStateManager::Destroy();

	SAFE_DELETE(m_ResCache);

	return 0;
}



//=========================================================
// GameCodeApp::FlashWhileMinimized - Chapter 5, page 149
//
// Wait for the application to be restored
// before going any further with the new 
// screen.  Flash until the person selects
// that they want to restore the game.
//
//=========================================================

void GameCodeApp::FlashWhileMinimized()
{
	// Flash the application on the taskbar
	// until it's restored.
	if ( ! GetHwnd() )
		return;
	
	// Blink the application if we are minimized,
	// waiting until we are no longer minimized
	if (IsIconic(GetHwnd()) )
	{
		// Make sure the app is up when creating a new screen
		// this should be the case most of the time, but when
		// we close the app down, minimized, and a confirmation
		// dialog appears, we need to restore
		DWORD now = timeGetTime();
		DWORD then = now;
		MSG msg;
		
		FlashWindow( GetHwnd(), true );
		
		while(true)
		{
			if ( PeekMessage( &msg, NULL, 0, 0, 0 ) )
			{
				if ( msg.message != WM_SYSCOMMAND || msg.wParam != SC_CLOSE )
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				
				// Are we done?
				if ( ! IsIconic(GetHwnd()) )
				{
					FlashWindow( GetHwnd(), false );
					break;
				}
			}
			else
			{
				now = timeGetTime();
				DWORD timeSpan = now > then ? (now - then) : (then - now);
				if ( timeSpan > 1000 )
				{
					then = now;
					FlashWindow( GetHwnd(), true );
				}
			}
		}
	}
}



//=========================================================
// GameCodeApp::OnAltEnter
//
// Called when the player hits Alt-Enter to flip the 
// display mode.
//
// Not discussed in the book.
//=========================================================

LRESULT GameCodeApp::OnAltEnter()
{
	DXUTToggleFullScreen();
	return 0;
}

//
// GameCodeApp::GetHumanView()					- not described in the book
//
//    FUTURE WORK - This function should accept a player number for split screen games
//
HumanView* GameCodeApp::GetHumanView()
{
	HumanView *pView = NULL;
	for(GameViewList::iterator i=m_pGame->m_gameViews.begin(); i!=m_pGame->m_gameViews.end(); ++i)
	{
		if ((*i)->VGetType()==GameView_Human)
		{
			shared_ptr<IGameView> pIGameView(*i);
			pView = static_cast<HumanView *>(&*pIGameView);
			break;
		}
	}
	return pView;
}

//
// class GameCodeApp::Modal						- Chapter 10, page 293
//
int GameCodeApp::Modal(shared_ptr<IScreenElement> pModalScreen, int defaultAnswer)
{
	// If we're going to display a dialog box, we need a human view 
	// to interact with.

	// [mrmike] This bit of code was refactored post press into GameCodeApp::GetHumanView()
	/***
	HumanView *pView;
	for(GameViewList::iterator i=m_pGame->m_gameViews.begin(); i!=m_pGame->m_gameViews.end(); ++i)
	{
		if ((*i)->VGetType()==GameView_Human)
		{
			shared_ptr<IGameView> pIGameView(*i);
			pView = static_cast<HumanView *>(&*pIGameView);
			break;
		}
	}
	***/

	HumanView* pView = GetHumanView();

	if (!pView)
	{
		// Whoops! There's no human view attached.
		return defaultAnswer;
	}

	if (m_HasModalDialog & 0x10000000)
	{
		GCC_ASSERT(0 && "Too Many nested dialogs!");
		return defaultAnswer;
	}
	
	GCC_ASSERT(GetHwnd() != NULL && _T("Main Window is NULL!"));
	if ( ( GetHwnd() != NULL ) && IsIconic(GetHwnd()) )
	{
		FlashWhileMinimized();
	}

	m_HasModalDialog <<= 1;
	m_HasModalDialog |= 1;

	pView->VPushElement(pModalScreen);

	LPARAM lParam = 0;
	int result = PumpUntilMessage(g_MsgEndModal, NULL, &lParam);
	
	if (lParam != 0)
	{
		if (lParam==g_QuitNoPrompt)
			result = defaultAnswer;
		else	
			result = (int)lParam;
	}

	pView->VRemoveElement(pModalScreen);
	m_HasModalDialog >>= 1;

	return result;
}

//
// class GameCodeApp::PumpUntilMessage			- Chapter 10, page 295
//
int GameCodeApp::PumpUntilMessage (UINT msgEnd, WPARAM* pWParam, LPARAM* pLParam)
{
	int currentTime = timeGetTime();
	MSG msg;
	for ( ;; )
	{
		if ( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
		{
			if (msg.message == WM_CLOSE)
			{
				m_bQuitting = true;
				GetMessage(& msg, NULL, 0, 0);
				break;
			}
			else
			{
				// Default processing
				if (GetMessage(&msg, NULL, NULL, NULL))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}

				// Are we done?
				if ( msg.message == msgEnd)
					break;
			}
		}
		else
		{
			// Update the game views, but nothing else!
			// Remember this is a modal screen.
			if (m_pGame)
			{
				int timeNow = timeGetTime();
				int deltaMilliseconds = timeNow - currentTime;
				for(GameViewList::iterator i=m_pGame->m_gameViews.begin(); i!=m_pGame->m_gameViews.end(); ++i)
				{
					(*i)->VOnUpdate( deltaMilliseconds );
				}
				currentTime = timeNow;
				DXUTRender3DEnvironment();
			}
		}
	}
	if (pLParam)
		*pLParam = msg.lParam;
	if (pWParam)
		*pWParam = msg.wParam;

	return 0;
}


//This function removes all of a *SPECIFIC* type of message from the queue...
int	GameCodeApp::EatSpecificMessages( UINT msgType, optional<LPARAM> lParam, optional<WPARAM> wParam)
{
	bool done = false;

	while (!done)
	{
		MSG msg;

		if ( PeekMessage( &msg, NULL, msgType, msgType, PM_NOREMOVE ) )
		{
			bool valid = true;

			if (lParam.valid())
			{
				valid &= (*lParam==msg.lParam);
			}

			if (wParam.valid())
			{
				valid &= (*wParam==msg.wParam);
			}

			if (valid)
			{
				//Soak!
				GetMessage(& msg, NULL, msgType, msgType);
			}
			else
			{
				done = true;
			}
		}
		else
		{
			done = true;	//No more messages!
		}
	}

	return 0;
}



//--------------------------------------------------------------------------------------
// Create any D3D9 resources that won't live through a device reset (D3DPOOL_DEFAULT) 
// or that are tied to the back buffer size 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK GameCodeApp::OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice,
                                    const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

	if (g_pApp->m_Renderer)
	{
		V_RETURN ( g_pApp->m_Renderer->VOnRestore() );
	}

	if (g_pApp->m_pGame)
	{
		BaseGameLogic *pGame = g_pApp->m_pGame;
		for(GameViewList::iterator i=pGame->m_gameViews.begin(); i!=pGame->m_gameViews.end(); ++i)
		{
			V_RETURN ( (*i)->VOnRestore() );
		}
	}

    return S_OK;
}



GameCodeApp::Renderer GameCodeApp::GetRendererImpl()
{
	if (DXUTGetDeviceSettings().ver == DXUT_D3D9_DEVICE )
		return Renderer_D3D9;
	else
		return Renderer_D3D11;
	return Renderer_Unknown;
};


//--------------------------------------------------------------------------------------
// Rejects any D3D9 devices that aren't acceptable to the app by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK GameCodeApp::IsD3D9DeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat,
                                      D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    // Skip backbuffer formats that don't support alpha blending
    IDirect3D9* pD3D = DXUTGetD3D9Object();
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                                         AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
                                         D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
        return false;

    // No fallback defined by this app, so reject any device that 
    // doesn't support at least ps2.0
    if( pCaps->PixelShaderVersion < D3DPS_VERSION( 2, 0 ) )
        return false;

    return true;
}



//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9ResetDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK GameCodeApp::OnD3D9LostDevice(void* pUserContext )
{
    D3DRenderer::g_DialogResourceManager.OnD3D9LostDevice();

	if (g_pApp->m_pGame)
	{
		BaseGameLogic *pGame = g_pApp->m_pGame;
		for(GameViewList::iterator i=pGame->m_gameViews.begin(); i!=pGame->m_gameViews.end(); ++i)
		{
			(*i)->VOnLostDevice();
		}
	}
}


//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK GameCodeApp::IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK GameCodeApp::OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext )
{
    HRESULT hr;

    ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
    V_RETURN( D3DRenderer::g_DialogResourceManager.OnD3D11CreateDevice( pd3dDevice, pd3dImmediateContext ) );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK GameCodeApp::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                         const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( D3DRenderer::g_DialogResourceManager.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );

	if (g_pApp->m_pGame)
	{
		BaseGameLogic *pGame = g_pApp->m_pGame;
		for(GameViewList::iterator i=pGame->m_gameViews.begin(); i!=pGame->m_gameViews.end(); ++i)
		{
			(*i)->VOnRestore();
		}
	}

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK GameCodeApp::OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
                                 float fElapsedTime, void* pUserContext )
{
	BaseGameLogic *pGame = g_pApp->m_pGame;

	for(GameViewList::iterator i=pGame->m_gameViews.begin(),
		end=pGame->m_gameViews.end(); i!=end; ++i)
	{
		(*i)->VOnRender(fTime, fElapsedTime);
	}

	g_pApp->m_pGame->VRenderDiagnostics();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK GameCodeApp::OnD3D11ReleasingSwapChain( void* pUserContext )
{
    D3DRenderer::g_DialogResourceManager.OnD3D11ReleasingSwapChain();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK GameCodeApp::OnD3D11DestroyDevice( void* pUserContext )
{
    if (g_pApp->m_Renderer)  // [rez] Fix for multi-monitor issue when target monitor is portrait; posted by Kl1X
	    g_pApp->m_Renderer->VShutdown();
    D3DRenderer::g_DialogResourceManager.OnD3D11DestroyDevice();
	g_pApp->m_Renderer = shared_ptr<IRenderer>(NULL);
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D11 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK GameCodeApp::ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    if( pDeviceSettings->ver == DXUT_D3D9_DEVICE )
    {
        IDirect3D9* pD3D = DXUTGetD3D9Object();
        D3DCAPS9 Caps;
        pD3D->GetDeviceCaps( pDeviceSettings->d3d9.AdapterOrdinal, pDeviceSettings->d3d9.DeviceType, &Caps );

        // If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW 
        // then switch to SWVP.
        if( ( Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) == 0 ||
            Caps.VertexShaderVersion < D3DVS_VERSION( 1, 1 ) )
        {
            pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        }

        // Debugging vertex shaders requires either REF or software vertex processing 
        // and debugging pixel shaders requires REF.  
#ifdef DEBUG_VS
        if( pDeviceSettings->d3d9.DeviceType != D3DDEVTYPE_REF )
        {
            pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
            pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
            pDeviceSettings->d3d9.BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        }
#endif
#ifdef DEBUG_PS
        pDeviceSettings->d3d9.DeviceType = D3DDEVTYPE_REF;
#endif
    }

    // For the first device created if its a REF device, optionally display a warning dialog box
    static bool s_bFirstTime = true;
    if( s_bFirstTime )
    {
        s_bFirstTime = false;
        if( ( DXUT_D3D9_DEVICE == pDeviceSettings->ver && pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF ) ||
            ( DXUT_D3D11_DEVICE == pDeviceSettings->ver &&
            pDeviceSettings->d3d11.DriverType == D3D_DRIVER_TYPE_REFERENCE ) )
        {
            DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
        }

    }

    return true;
}





//--------------------------------------------------------------------------------------
// This callback function will be called once at the beginning of every frame. This is the
// best location for your application to handle updates to the scene, but is not 
// intended to contain actual rendering calls, which should instead be placed in the 
// OnFrameRender callback.  
//
// See Game Coding Complete - 4th Edition - Chapter X, page Y
//--------------------------------------------------------------------------------------
void CALLBACK GameCodeApp::OnUpdateGame( double fTime, float fElapsedTime, void* pUserContext  )
{
	if (g_pApp->HasModalDialog())
	{	
		// don't update the game if a modal dialog is up.
		return;
	}

	if (g_pApp->m_bQuitting)
	{
		PostMessage(g_pApp->GetHwnd(), WM_CLOSE, 0, 0);
	}

	if (g_pApp->m_pGame)
	{
        IEventManager::Get()->VUpdate(20); // allow event queue to process for up to 20 ms

		if (g_pApp->m_pBaseSocketManager)
			g_pApp->m_pBaseSocketManager->DoSelect(0);	// pause 0 microseconds

		g_pApp->m_pGame->VOnUpdate(float(fTime), fElapsedTime);
	}
}






//--------------------------------------------------------------------------------------
// This callback function will be called at the end of every frame to perform all the 
// rendering calls for the scene, and it will also be called if the window needs to be 
// repainted. After this function has returned, the sample framework will call 
// IDirect3DDevice9::Present to display the contents of the next buffer in the swap chain
//
// See Game Coding Complete - 3rd Edition - Chapter 6 - page 154
//--------------------------------------------------------------------------------------
void CALLBACK GameCodeApp::OnD3D9FrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext  )
{
	BaseGameLogic *pGame = g_pApp->m_pGame;

	for(GameViewList::iterator i=pGame->m_gameViews.begin(),
		end=pGame->m_gameViews.end(); i!=end; ++i)
	{
		(*i)->VOnRender(fTime, fElapsedTime);
	}

	g_pApp->m_pGame->VRenderDiagnostics();
}


//--------------------------------------------------------------------------------------
// Create any D3D9 resources that will live through a device reset (D3DPOOL_MANAGED)
// and aren't tied to the back buffer size
//--------------------------------------------------------------------------------------
HRESULT CALLBACK GameCodeApp::OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext )
{
    HRESULT hr;

    V_RETURN( D3DRenderer::g_DialogResourceManager.OnD3D9CreateDevice( pd3dDevice ) );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9CreateDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK GameCodeApp::OnD3D9DestroyDevice( void* pUserContext )
{
	g_pApp->m_Renderer->VShutdown();
	D3DRenderer::g_DialogResourceManager.OnD3D9DestroyDevice( );
	g_pApp->m_Renderer = shared_ptr<IRenderer>(NULL);
}


bool GameCodeApp::AttachAsClient()
{
	ClientSocketManager *pClient = GCC_NEW ClientSocketManager(g_pApp->m_Options.m_gameHost, g_pApp->m_Options.m_listenPort);
	if (!pClient->Connect())
	{
		return false;
	}
	g_pApp->m_pBaseSocketManager = pClient;
	VCreateNetworkEventForwarder();

	return true;
}


// Any events that will be received from the server logic should be here!
void GameCodeApp::VCreateNetworkEventForwarder(void)
{
    if (m_pNetworkEventForwarder != NULL)
    {
        GCC_ERROR("Overwriting network event forwarder in TeapotWarsApp!");
        SAFE_DELETE(m_pNetworkEventForwarder);
    }

    m_pNetworkEventForwarder = GCC_NEW NetworkEventForwarder(0);

    IEventManager* pGlobalEventManager = IEventManager::Get();
	pGlobalEventManager->VAddListener(MakeDelegate(m_pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Request_New_Actor::sk_EventType);
	pGlobalEventManager->VAddListener(MakeDelegate(m_pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Environment_Loaded::sk_EventType);
	pGlobalEventManager->VAddListener(MakeDelegate(m_pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_PhysCollision::sk_EventType);

}

void GameCodeApp::VDestroyNetworkEventForwarder(void)
{
    if (m_pNetworkEventForwarder)
    {
        IEventManager* pGlobalEventManager = IEventManager::Get();
		pGlobalEventManager->VRemoveListener(MakeDelegate(m_pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Request_New_Actor::sk_EventType);
		pGlobalEventManager->VRemoveListener(MakeDelegate(m_pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Environment_Loaded::sk_EventType);
		pGlobalEventManager->VRemoveListener(MakeDelegate(m_pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_PhysCollision::sk_EventType);
        SAFE_DELETE(m_pNetworkEventForwarder);
    }
}


