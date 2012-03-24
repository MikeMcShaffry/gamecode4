//========================================================================
// TeapotWars.cpp : source file for the sample game
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

#include "TeapotWarsStd.h"

#include "../GCC4/GameCode4/GameCode.h"
#include "../GCC4/Audio/Audio.h"
#include "../GCC4/Audio/SoundProcess.h"
#include "../GCC4/Graphics3D/D3DRenderer.h"
#include "../GCC4/Graphics3D/Geometry.h"
#include "../GCC4/Graphics3D/MovementController.h"
#include "../GCC4/Graphics3D/Raycast.h"
#include "../GCC4/Graphics3D/SceneNodes.h"
#include "../GCC4/Graphics3D/Sky.h"
#include "../GCC4/MainLoop/Initialization.h"
#include "../GCC4/EventManager/Events.h"
#include "../GCC4/EventManager/EventManagerImpl.h"
#include "../GCC4/Physics/PhysicsEventListener.h"
#include "../GCC4/UserInterface/HumanView.h"
#include "../GCC4/UserInterface/MessageBox.h"
#include "../GCC4/Actors/Actor.h"
#include "../GCC4/Actors/RenderComponent.h"
#include "../GCC4/Utilities/String.h"
#include "../GCC4/LUAScripting/LuaStateManager.h"  // [rez] For debugging only; the view shouldn't use Lua

#include "TeapotWarsView.h"
#include "TeapotWarsNetwork.h"
#include "TeapotEvents.h"

#include "TeapotController.h"
#include "TeapotWars.h"

extern TeapotWarsApp g_TeapotWarsApp;

//========================================================================
//
// MainMenuUI & MainMenuView implementation
//
//
//========================================================================

#define CID_CREATE_GAME_RADIO			(2)
#define CID_JOIN_GAME_RADIO				(3)
#define CID_NUM_AI_SLIDER				(4)
#define CID_NUM_PLAYER_SLIDER			(5)
#define CID_HOST_LISTEN_PORT			(6)
#define CID_CLIENT_ATTACH_PORT			(7)
#define CID_START_BUTTON				(8)
#define CID_HOST_NAME					(9)
#define CID_NUM_AI_LABEL				(10)
#define CID_NUM_PLAYER_LABEL			(11)
#define CID_HOST_LISTEN_PORT_LABEL		(12)
#define CID_CLIENT_ATTACH_PORT_LABEL	(13)
#define CID_HOST_NAME_LABEL				(14)
#define CID_LEVEL_LABEL					(15)
#define CID_LEVEL_LISTBOX				(16)


const int g_SampleUIWidth = 600;
const int g_SampleUIHeight = 600;

MainMenuUI::MainMenuUI()
{
	// This line was added to comply with DirectX 9c (June 2005) 
	m_SampleUI.Init( &D3DRenderer::g_DialogResourceManager );
    m_SampleUI.SetCallback( OnGUIEvent, this ); 

	int iY = 10; 
	int iX = 35;
	int iX2 = g_SampleUIWidth / 2;
	int width = (g_SampleUIWidth/2)-10;
	int height = 25;
	int lineHeight = height + 2;

	// grab defaults from the game options.
	m_NumAIs = g_pApp->m_Options.m_numAIs;
	m_NumPlayers = g_pApp->m_Options.m_expectedPlayers;
	m_HostName = g_pApp->m_Options.m_gameHost;
	m_HostListenPort = g_pApp->m_Options.m_listenPort;
	m_ClientAttachPort = g_pApp->m_Options.m_listenPort;

	m_bCreatingGame = true;

	D3DCOLOR color = 0x50505050;
	m_SampleUI.SetBackgroundColors(color);

	m_SampleUI.SetFont(0, L"Ariel", height, 0);

	m_SampleUI.AddStatic(0, L"Teapot Wars Main Menu", iX-20, iY, g_SampleUIWidth, height * 2);
	iY += (lineHeight * 3);

    m_SampleUI.AddRadioButton( CID_CREATE_GAME_RADIO, 1, L"Create Game", iX, iY, g_SampleUIWidth, height);
	iY += lineHeight;

	m_SampleUI.AddStatic(CID_LEVEL_LABEL, L"Level", iX, iY, width, height);
	m_SampleUI.AddListBox(CID_LEVEL_LISTBOX, iX2, iY, width, lineHeight*5);
	std::vector<Level> levels = g_pApp->GetGameLogic()->GetLevelManager()->GetLevels();
	m_Levels.reserve(levels.size());
	int count = 0;
	for (std::vector<Level>::iterator i = levels.begin(); i != levels.end(); ++i, ++count)
	{
		m_Levels.push_back(s2ws(*i));
		m_SampleUI.GetListBox ( CID_LEVEL_LISTBOX )->AddItem(m_Levels[count].c_str(), NULL);
	}
	iY += (lineHeight * 5);
	//m_SampleUI.GetListBox(CID_LEVEL_LISTBOX)->GetElement(0)->SetFont(0, 0x0);

	m_SampleUI.AddStatic(CID_NUM_AI_LABEL, L"", iX, iY, width, height);
	m_SampleUI.AddSlider( CID_NUM_AI_SLIDER, iX2, iY, width, height);
	m_SampleUI.GetSlider( CID_NUM_AI_SLIDER )->SetRange(0, g_pApp->m_Options.m_maxAIs);
	m_SampleUI.GetSlider( CID_NUM_AI_SLIDER )->SetValue(m_NumAIs); // should be ai options default
	iY += lineHeight;

	m_SampleUI.AddStatic(CID_NUM_PLAYER_LABEL, L"", iX, iY, width, height);
	m_SampleUI.AddSlider( CID_NUM_PLAYER_SLIDER, iX2, iY, width, height);
	m_SampleUI.GetSlider( CID_NUM_PLAYER_SLIDER )->SetRange(1, g_pApp->m_Options.m_maxPlayers);
	m_SampleUI.GetSlider( CID_NUM_PLAYER_SLIDER )->SetValue(m_NumPlayers);  // should be player options default
	iY += lineHeight;

	m_SampleUI.AddStatic(CID_HOST_LISTEN_PORT_LABEL, L"Host Listen Port", iX, iY, width, height);
	m_SampleUI.AddEditBox( CID_HOST_LISTEN_PORT, L"57", iX2, iY, width, height * 2);
	CDXUTEditBox *eb = m_SampleUI.GetEditBox( CID_HOST_LISTEN_PORT );
	eb->SetVisible(false);
	iY += lineHeight * 3;

    m_SampleUI.AddRadioButton( CID_JOIN_GAME_RADIO, 1, L"Join Game", iX, iY, width, height);
    m_SampleUI.GetRadioButton( CID_JOIN_GAME_RADIO )->SetChecked( true ); 
	iY += lineHeight;

	m_SampleUI.AddStatic(CID_CLIENT_ATTACH_PORT_LABEL, L"Host Attach Port", iX, iY, width, height);
	m_SampleUI.AddEditBox( CID_CLIENT_ATTACH_PORT, L"57", iX2, iY, width, height * 2);
	iY += lineHeight * 3;


	m_SampleUI.AddStatic(CID_HOST_NAME_LABEL, L"Host Name", iX, iY, width, height);
	m_SampleUI.AddEditBox( CID_HOST_NAME, L"sunshine", iX2, iY, width, height * 2);
	iY += lineHeight;

	m_SampleUI.AddButton(CID_START_BUTTON, L"Start Game",  (g_SampleUIWidth-(width/2))/2, iY += lineHeight, width/2, height);

    m_SampleUI.GetRadioButton( CID_CREATE_GAME_RADIO )->SetChecked(true);

	Set();
}


MainMenuUI::~MainMenuUI()
{ 
	D3DRenderer::g_DialogResourceManager.UnregisterDialog(&m_SampleUI);
}

void MainMenuUI::Set()
{
	WCHAR buffer[256];
	CHAR ansiBuffer[256];

	m_LevelIndex = m_SampleUI.GetListBox ( CID_LEVEL_LISTBOX)->GetSelectedIndex();
	m_SampleUI.GetListBox ( CID_LEVEL_LISTBOX)->SetVisible(m_bCreatingGame);

	m_NumAIs = m_SampleUI.GetSlider( CID_NUM_AI_SLIDER )->GetValue(); 
	m_SampleUI.GetSlider(CID_NUM_AI_SLIDER)->SetVisible(m_bCreatingGame);

	wsprintf( buffer, _T("%s: %d\n"), L"Number of AIs", m_NumAIs );
	m_SampleUI.GetStatic(CID_NUM_AI_LABEL)->SetText(buffer);
	m_SampleUI.GetStatic(CID_NUM_AI_LABEL)->SetVisible(m_bCreatingGame);
	
	m_NumPlayers = m_SampleUI.GetSlider( CID_NUM_PLAYER_SLIDER )->GetValue(); 
	m_SampleUI.GetSlider(CID_NUM_PLAYER_SLIDER)->SetVisible(m_bCreatingGame);
	wsprintf( buffer, _T("%s: %d\n"), L"Number of Players", m_NumPlayers );
	m_SampleUI.GetStatic(CID_NUM_PLAYER_LABEL)->SetText(buffer);
	m_SampleUI.GetStatic(CID_NUM_PLAYER_LABEL)->SetVisible(m_bCreatingGame);

	m_SampleUI.GetStatic( CID_HOST_LISTEN_PORT_LABEL )->SetVisible(m_NumPlayers>1 && m_bCreatingGame);
	m_SampleUI.GetEditBox( CID_HOST_LISTEN_PORT )->SetVisible(m_NumPlayers>1 && m_bCreatingGame);
	if (m_bCreatingGame)
	{
		WideToAnsiCch(ansiBuffer, m_SampleUI.GetEditBox( CID_HOST_LISTEN_PORT )->GetText(), 256);
		m_HostListenPort = ansiBuffer;
	}

	m_SampleUI.GetStatic( CID_HOST_NAME_LABEL )->SetVisible(!m_bCreatingGame);
	m_SampleUI.GetEditBox( CID_HOST_NAME )->SetVisible(!m_bCreatingGame);

	WideToAnsiCch(ansiBuffer, m_SampleUI.GetEditBox( CID_HOST_NAME )->GetText(), 256);
	m_HostName = ansiBuffer;

	m_SampleUI.GetStatic( CID_CLIENT_ATTACH_PORT_LABEL )->SetVisible(!m_bCreatingGame);
	m_SampleUI.GetEditBox( CID_CLIENT_ATTACH_PORT )->SetVisible(!m_bCreatingGame);
	if (!m_bCreatingGame)
	{
		WideToAnsiCch(ansiBuffer, m_SampleUI.GetEditBox( CID_CLIENT_ATTACH_PORT )->GetText(), 256);
		m_ClientAttachPort = ansiBuffer;
	}

}

HRESULT MainMenuUI::VOnRestore()
{
    m_SampleUI.SetLocation( (g_pApp->GetScreenSize().x - g_SampleUIWidth)/2, (g_pApp->GetScreenSize().y - g_SampleUIHeight) / 2  );
    m_SampleUI.SetSize( g_SampleUIWidth, g_SampleUIHeight );
	return S_OK;
}

HRESULT MainMenuUI::VOnRender(double fTime, float fElapsedTime)
{
	HRESULT hr;
	DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"TeapotWarsHUD" ); // These events are to help PIX identify what the code is doing
	V( m_SampleUI.OnRender( fElapsedTime ) );
	DXUT_EndPerfEvent();
	return S_OK;
};

LRESULT CALLBACK MainMenuUI::VOnMsgProc( AppMsg msg )
{
	return m_SampleUI.MsgProc( msg.m_hWnd, msg.m_uMsg, msg.m_wParam, msg.m_lParam );
}



//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
//    Note: pUserContext added to comply with DirectX 9c - June 2005 Update
//
void CALLBACK MainMenuUI::_OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void *pUserContext )
{
    switch (nControlID)
    {
		case CID_CREATE_GAME_RADIO:
        {
			m_bCreatingGame = true;
			break;
        }

		case CID_JOIN_GAME_RADIO:
        {
			m_bCreatingGame = false;
			break;
        }

		case CID_LEVEL_LISTBOX:
		case CID_NUM_AI_SLIDER:
		case CID_NUM_PLAYER_SLIDER:
		case CID_HOST_LISTEN_PORT:
		case CID_CLIENT_ATTACH_PORT:
		case CID_HOST_NAME:
        {
			break;
        }

		case CID_START_BUTTON:
        {
			g_pApp->m_Options.m_numAIs = m_NumAIs;
			g_pApp->m_Options.m_expectedPlayers = m_NumPlayers;
			if (m_bCreatingGame)
			{
				if (m_LevelIndex == -1)
				{
					// FUTURE WORK - AN ERROR DIALOG WOULD BE GOOD HERE, OR JUST DEFALT THE SELECTION TO SOMETHING VALID
					return;
				}
		        g_pApp->m_Options.m_Level = ws2s(m_Levels[m_LevelIndex]);
				g_pApp->m_Options.m_gameHost = "";
				g_pApp->m_Options.m_listenPort = atoi(m_HostListenPort.c_str());
			}
			else
			{
				g_pApp->m_Options.m_gameHost = m_HostName;
				g_pApp->m_Options.m_listenPort = atoi(m_ClientAttachPort.c_str());
			}

			VSetVisible(false);

            shared_ptr<EvtData_Request_Start_Game> pRequestStartGameEvent(GCC_NEW EvtData_Request_Start_Game());
            IEventManager::Get()->VQueueEvent(pRequestStartGameEvent);

			break;
        }

		default:
        {
			GCC_ERROR("Unknown control.");
        }
    }

	Set();
}

void CALLBACK MainMenuUI::OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void *pUserContext )
{
	MainMenuUI *ui = static_cast<MainMenuUI *>(pUserContext);
	ui->_OnGUIEvent(nEvent, nControlID, pControl, pUserContext);
}


MainMenuView::MainMenuView() : HumanView(shared_ptr<IRenderer>())
{
	m_MainMenuUI.reset(GCC_NEW MainMenuUI); 
	VPushElement(m_MainMenuUI);
}


MainMenuView::~MainMenuView()
{
}


void MainMenuView::VRenderText()
{
	HumanView::VRenderText();
}


void MainMenuView::VOnUpdate(unsigned long deltaMs)
{
	HumanView::VOnUpdate( deltaMs );
}

LRESULT CALLBACK MainMenuView::VOnMsgProc( AppMsg msg )
{
	if (m_MainMenuUI->VIsVisible() )
	{
		if (HumanView::VOnMsgProc(msg))
			return 1;
	}
	return 0;
}


//========================================================================
//
// StandardHUD implementation
//
//
//========================================================================

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           2
#define IDC_CHANGEDEVICE        3

//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
//    Note: pUserContext added to comply with DirectX 9c - June 2005 Update
//
void CALLBACK StandardHUD::OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void *pUserContext )
{
    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN: DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:        DXUTToggleREF(); break;
//        case IDC_CHANGEDEVICE:     DXUTSetShowSettingsDialog( !DXUTGetShowSettingsDialog() ); break;
    }
}


StandardHUD::StandardHUD()
{
    // Initialize dialogs
	m_HUD.Init( &D3DRenderer::g_DialogResourceManager );
    m_HUD.SetCallback( OnGUIEvent ); int iY = 10; 
    m_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, iY, 125, 22 );
    m_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 35, iY += 24, 125, 22 );
    //m_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 35, iY += 24, 125, 22 );
}


StandardHUD::~StandardHUD() 
{ 
//  [mrmike] - this causes a "memory written after freed error" so I commented it out.
//	D3DRenderer::g_DialogResourceManager.UnregisterDialog(&m_HUD); 
}


HRESULT StandardHUD::VOnRestore()
{
    m_HUD.SetLocation( g_pApp->GetScreenSize().x - 170, 0 );
    m_HUD.SetSize( 170, 170 );
	return S_OK;
}



HRESULT StandardHUD::VOnRender(double fTime, float fElapsedTime)
{
	HRESULT hr;
	DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"StandardUI" ); // These events are to help PIX identify what the code is doing
	V( m_HUD.OnRender( fElapsedTime ) );
	DXUT_EndPerfEvent();
	return S_OK;
};


LRESULT CALLBACK StandardHUD::VOnMsgProc( AppMsg msg )
{
    return m_HUD.MsgProc( msg.m_hWnd, msg.m_uMsg, msg.m_wParam, msg.m_lParam );
}



//========================================================================
//
// TeapotWarsHumanView Implementation
//
//========================================================================

//
// TeapotWarsHumanView::TeapotWarsHumanView	- Chapter 19, page 724
//
TeapotWarsHumanView::TeapotWarsHumanView(shared_ptr<IRenderer> renderer) : HumanView(renderer)
{ 
	m_bShowUI = true; 
    RegisterAllDelegates();
}


TeapotWarsHumanView::~TeapotWarsHumanView() 
{
    RemoveAllDelegates();
}

//
// TeapotWarsHumanView::VOnMsgProc				- Chapter 19, page 727
//
LRESULT CALLBACK TeapotWarsHumanView::VOnMsgProc( AppMsg msg )
{
	if (HumanView::VOnMsgProc(msg))
		return 1;

	if (msg.m_uMsg==WM_KEYDOWN)
	{
		if (msg.m_wParam==VK_F1)
		{
			m_bShowUI = !m_bShowUI;
			//m_StandardHUD->VSetVisible(m_bShowUI);
			return 1;
		}
		else if (msg.m_wParam==VK_F2)
		{
			// test the picking API

			POINT ptCursor;
			GetCursorPos( &ptCursor );
			ScreenToClient( g_pApp->GetHwnd(), &ptCursor );

			RayCast rayCast(ptCursor);
			m_pScene->Pick(&rayCast);
			rayCast.Sort();

			if (rayCast.m_NumIntersections)
			{
				// You can iterate through the intersections on the raycast.
				int a = 0;
			}
		}
		else if (msg.m_wParam==VK_F3)
		{
			//extern void CreateThreads();
			//CreateThreads();

			extern void testThreading(ProcessManager *procMgr, bool runProtected);
			testThreading(m_pProcessManager, true);

			return 1;

			Sleep(5000);
			testThreading(m_pProcessManager, false);
			Sleep(5000);

			extern void testRealtimeEvents(ProcessManager *procMgr);
			testRealtimeEvents(m_pProcessManager);

			extern void testRealtimeDecompression(ProcessManager *procMgr);
			testRealtimeDecompression(m_pProcessManager);
		}
        else if (msg.m_wParam == VK_F4)
        {
            Resource resource("scripts\\test.lua");
            shared_ptr<ResHandle> pResourceHandle = g_pApp->m_ResCache->GetHandle(&resource);  // this actually loads the Lua file from the zip file
        }
        else if (msg.m_wParam == VK_F5)
        {
            shared_ptr<EvtData_ScriptEventTest_ToLua> pEvent(GCC_NEW EvtData_ScriptEventTest_ToLua);
            IEventManager::Get()->VQueueEvent(pEvent);
        }
        else if (msg.m_wParam == VK_F6)
        {
            LuaStateManager::Get()->VExecuteString("TestProcess()");
        }
		else if (msg.m_wParam==VK_F8)
		{
			TeapotWarsLogic *twg = static_cast<TeapotWarsLogic *>(g_pApp->m_pGame);
			twg->ToggleRenderDiagnostics();

		}
		else if (msg.m_wParam==VK_F9)
		{
			m_KeyboardHandler = m_pTeapotController;
			m_PointerHandler = m_pTeapotController;
			m_pCamera->SetTarget(m_pTeapot);
			m_pTeapot->SetAlpha(0.8f);
			ReleaseCapture();
			return 1;
		}
		else if (msg.m_wParam==VK_F11)
		{
			m_KeyboardHandler = m_pFreeCameraController;
			m_PointerHandler = m_pFreeCameraController;
			m_pCamera->ClearTarget();
			//m_pTeapot->SetAlpha(fOPAQUE);
			SetCapture(g_pApp->GetHwnd());
			return 1;
		}
		else if (msg.m_wParam=='Q' || msg.m_wParam==VK_ESCAPE)				// Jan 2010 - mlm - added VK_ESCAPE since it is on the onscreen help!
		{
			if (MessageBox::Ask(QUESTION_QUIT_GAME)==IDYES)
			{
				g_pApp->SetQuitting(true);
			}
			return 1;
		}
	}

	return 0;
}


//
// TeapotWarsHumanView::VRenderText				- Chapter 19, page 727
//
void TeapotWarsHumanView::VRenderText()
{
	if (!D3DRenderer::g_pTextHelper)
		return;

	HumanView::VRenderText();

	D3DRenderer::g_pTextHelper->Begin();

    // Gameplay UI (with shadow)....
    if (!m_gameplayText.empty())
    {
	    D3DRenderer::g_pTextHelper->SetInsertionPos( g_pApp->GetScreenSize().x/2, 5 );
	    D3DRenderer::g_pTextHelper->SetForegroundColor( D3DXCOLOR( 0.0f, 0.0f, 0.0f, 1.0f ) );
	    D3DRenderer::g_pTextHelper->DrawTextLine(m_gameplayText.c_str());
	    D3DRenderer::g_pTextHelper->SetInsertionPos( g_pApp->GetScreenSize().x/2-1, 5-1 );
	    D3DRenderer::g_pTextHelper->SetForegroundColor( D3DXCOLOR( 0.25f, 1.0f, 0.25f, 1.0f ) );
	    D3DRenderer::g_pTextHelper->DrawTextLine(m_gameplayText.c_str());
    }
	// ...Gameplay UI

	if( m_bShowUI )
	{
		// Output statistics...
		D3DRenderer::g_pTextHelper->SetInsertionPos( 5, 5 );
		D3DRenderer::g_pTextHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
		D3DRenderer::g_pTextHelper->DrawTextLine( DXUTGetFrameStats() );
		D3DRenderer::g_pTextHelper->DrawTextLine( DXUTGetDeviceStats() );
		//...output statistics
		
		D3DRenderer::g_pTextHelper->SetForegroundColor( D3DXCOLOR( 0.0f, 0.0f, 0.0f, 0.5f ) );

		//Game State...
		switch (m_BaseGameState)
		{
			case BGS_Initializing:
				D3DRenderer::g_pTextHelper->DrawTextLine(g_pApp->GetString(_T("IDS_INITIALIZING")).c_str());
				break;

			case BGS_MainMenu:
				D3DRenderer::g_pTextHelper->DrawTextLine(L"Main Menu");
				break;

//			case BGS_SpawnAI:
//				D3DRenderer::g_pTextHelper->DrawTextLine(L"Spawn AI");
//				break;

			case BGS_WaitingForPlayers:
				D3DRenderer::g_pTextHelper->DrawTextLine(g_pApp->GetString(_T("IDS_WAITING")).c_str());
				break;

			case BGS_LoadingGameEnvironment:
				D3DRenderer::g_pTextHelper->DrawTextLine(g_pApp->GetString(_T("IDS_LOADING")).c_str());
				break;

			case BGS_Running:
#ifndef DISABLE_PHYSICS
				D3DRenderer::g_pTextHelper->DrawTextLine(g_pApp->GetString(_T("IDS_RUNNING")).c_str());
#else
				D3DRenderer::g_pTextHelper->DrawTextLine(g_pApp->GetString(_T("IDS_NOPHYSICS")).c_str());
#endif //!DISABLE_PHYSICS
				break;
		}
		//...Game State

		//Camera...
		TCHAR buffer[256];
		const TCHAR *s = NULL;
		Mat4x4 toWorld;
		Mat4x4 fromWorld;
		if (m_pCamera)
		{	
			m_pCamera->VGet()->Transform(&toWorld, &fromWorld);
		}
		swprintf(buffer, g_pApp->GetString(_T("IDS_CAMERA_LOCATION")).c_str(), toWorld.m[3][0], toWorld.m[3][1], toWorld.m[3][2]);
		D3DRenderer::g_pTextHelper->DrawTextLine( buffer );
		//...Camera

		//Help text.  Right justified, lower right of screen.
		RECT helpRect;
		helpRect.left = 0;
		helpRect.right = g_pApp->GetScreenSize().x - 10;
		helpRect.top = g_pApp->GetScreenSize().y - 15*8;
		helpRect.bottom = g_pApp->GetScreenSize().y;
		D3DRenderer::g_pTextHelper->SetInsertionPos( helpRect.right, helpRect.top );
		D3DRenderer::g_pTextHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 0.75f, 0.0f, 1.0f ) );
		D3DRenderer::g_pTextHelper->DrawTextLine( helpRect, DT_RIGHT, g_pApp->GetString(_T("IDS_CONTROLS_HEADER")).c_str() );
		helpRect.top = g_pApp->GetScreenSize().y-15*7;
		D3DRenderer::g_pTextHelper->DrawTextLine( helpRect, DT_RIGHT, g_pApp->GetString(_T("IDS_CONTROLS")).c_str() );
		//...Help
	}//end if (m_bShowUI)

	D3DRenderer::g_pTextHelper->End();
}



//
// TeapotWarsHumanView::VOnUpdate				- Chapter 19, page 730
//
void TeapotWarsHumanView::VOnUpdate(unsigned long deltaMs)
{
	HumanView::VOnUpdate( deltaMs );

	if (m_pFreeCameraController)
	{
		m_pFreeCameraController->OnUpdate(deltaMs);
	}

	if (m_pTeapotController)
	{
		m_pTeapotController->OnUpdate(deltaMs);
	}

	//Send out a tick to listeners.
	shared_ptr<EvtData_Update_Tick> pTickEvent(GCC_NEW EvtData_Update_Tick(deltaMs));
    IEventManager::Get()->VTriggerEvent(pTickEvent);
}

//
// TeapotWarsHumanView::VOnAttach				- Chapter 19, page 731
//
void TeapotWarsHumanView::VOnAttach(GameViewId vid, ActorId aid)
{
	HumanView::VOnAttach(vid, aid);
}

bool TeapotWarsHumanView::VLoadGameDelegate(TiXmlElement* pLevelData)
{
	if (!HumanView::VLoadGameDelegate(pLevelData))
		return false;

    m_StandardHUD.reset(GCC_NEW StandardHUD); 
    VPushElement(m_StandardHUD);

    // A movement controller is going to control the camera, 
    // but it could be constructed with any of the objects you see in this
    // function. You can have your very own remote controlled sphere. What fun...
    m_pFreeCameraController.reset(GCC_NEW MovementController(m_pCamera, 0, 0, false));

    m_pScene->VOnRestore();
    return true;
}

void TeapotWarsHumanView::VSetControlledActor(ActorId actorId) 
{ 
	m_pTeapot = static_pointer_cast<SceneNode>(m_pScene->FindActor(actorId));
    if (!m_pTeapot)
    {
        GCC_ERROR("Invalid teapot");
        return;
    }

	HumanView::VSetControlledActor(actorId);

    m_pTeapotController.reset(GCC_NEW TeapotController(m_pTeapot));
    m_KeyboardHandler = m_pTeapotController;
    m_PointerHandler = m_pTeapotController;
    m_pCamera->SetTarget(m_pTeapot);
    m_pTeapot->SetAlpha(0.8f);
}

void TeapotWarsHumanView::GameplayUiUpdateDelegate(IEventDataPtr pEventData)
{
    shared_ptr<EvtData_Gameplay_UI_Update> pCastEventData = static_pointer_cast<EvtData_Gameplay_UI_Update>(pEventData);
    if (!pCastEventData->GetUiString().empty())
        m_gameplayText = s2ws(pCastEventData->GetUiString());
    else
        m_gameplayText.clear();
}

void TeapotWarsHumanView::SetControlledActorDelegate(IEventDataPtr pEventData)
{
    shared_ptr<EvtData_Set_Controlled_Actor> pCastEventData = static_pointer_cast<EvtData_Set_Controlled_Actor>(pEventData);
    VSetControlledActor(pCastEventData->GetActorId());
}


void TeapotWarsHumanView::RegisterAllDelegates(void)
{
	// [mrmike] Move, New, and Destroy actor events are now handled by the HumanView, as are the PlaySound and GameState events

    IEventManager* pGlobalEventManager = IEventManager::Get();
    pGlobalEventManager->VAddListener(MakeDelegate(this, &TeapotWarsHumanView::GameplayUiUpdateDelegate), EvtData_Gameplay_UI_Update::sk_EventType);
    pGlobalEventManager->VAddListener(MakeDelegate(this, &TeapotWarsHumanView::SetControlledActorDelegate), EvtData_Set_Controlled_Actor::sk_EventType);
}

void TeapotWarsHumanView::RemoveAllDelegates(void)
{
    IEventManager* pGlobalEventManager = IEventManager::Get();
    pGlobalEventManager->VRemoveListener(MakeDelegate(this, &TeapotWarsHumanView::GameplayUiUpdateDelegate), EvtData_Gameplay_UI_Update::sk_EventType);
    pGlobalEventManager->VRemoveListener(MakeDelegate(this, &TeapotWarsHumanView::SetControlledActorDelegate), EvtData_Set_Controlled_Actor::sk_EventType);
}



///////////////////////////////////////////////////////////////////////////////
//
// AITeapotView::AITeapotView					- Chapter 19, page 737
//
AITeapotView::AITeapotView(shared_ptr<PathingGraph> pPathingGraph) : IGameView(), m_pPathingGraph(pPathingGraph)
{
    //
}

//
// AITeapotView::~AITeapotView					- Chapter 19, page 737
//
AITeapotView::~AITeapotView(void)
{
    GCC_LOG("AI", "Destroying AITeapotView");
}
