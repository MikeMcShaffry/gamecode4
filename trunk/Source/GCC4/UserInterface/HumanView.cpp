//========================================================================
// HumanView.cpp - Implements the class HumanView, which provides a Human interface into the game
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
#include <DXUTGui.h>			// for the dialog resource manager and the text helper class.

#include "HumanView.h"

#include "../Actors/AudioComponent.h"
#include "../Actors/RenderComponentInterface.h"
#include "../Audio/DirectSoundAudio.h"
#include "../Audio/SoundProcess.h"
#include "../EventManager/Events.h"
#include "../EventManager/EventManagerImpl.h"
#include "../Graphics3D/D3DRenderer.h"
#include "../Graphics3D/Scene.h"
#include "../MainLoop/Process.h"
#include "../Utilities/String.h"
#include "../LUAScripting/LuaStateManager.h"

const unsigned int SCREEN_REFRESH_RATE(1000/60);
const GameViewId gc_InvalidGameViewId = 0xffffffff;

//
// HumanView::HumanView - Chapter 10, page 272
//
HumanView::HumanView(shared_ptr<IRenderer> renderer)
{
	InitAudio(); 

	m_pProcessManager = GCC_NEW ProcessManager;

	m_PointerRadius = 1;	// we assume we are on a mouse enabled machine - if this were a tablet we should detect it here.
	m_ViewId = gc_InvalidGameViewId;

	// Added post press for move, new, and destroy actor events and others
	RegisterAllDelegates();
	m_BaseGameState = BGS_Initializing;		// what is the current game state

	if (renderer)
	{
		// Moved to the HumanView class post press
		m_pScene.reset(GCC_NEW ScreenElementScene(renderer));

		Frustum frustum;
		frustum.Init(GCC_PI/4.0f, 1.0f, 1.0f, 100.0f);
		m_pCamera.reset(GCC_NEW CameraNode(&Mat4x4::g_Identity, frustum));
		GCC_ASSERT(m_pScene && m_pCamera && _T("Out of memory"));

		m_pScene->VAddChild(INVALID_ACTOR_ID, m_pCamera);
		m_pScene->SetCamera(m_pCamera);
	}
}


HumanView::~HumanView()
{
	// [mrmike] - RemoveAllDelegates was added post press to handle move, new, and destroy actor events.
	RemoveAllDelegates();

	while (!m_ScreenElements.empty())
	{
		m_ScreenElements.pop_front();		
	}

	SAFE_DELETE(m_pProcessManager);

	SAFE_DELETE(g_pAudio);
}

bool HumanView::LoadGame(TiXmlElement* pLevelData)
{
    // call the delegate method
    return VLoadGameDelegate(pLevelData);
}

//
// HumanView::VOnRender							- Chapter 10, page 274
//
void HumanView::VOnRender(double fTime, float fElapsedTime )
{
	m_currTick = timeGetTime();
	if (m_currTick == m_lastDraw)
		return;

	// It is time to draw ?
	if( m_runFullSpeed || ( (m_currTick - m_lastDraw) > SCREEN_REFRESH_RATE) )
	{
		if (g_pApp->m_Renderer->VPreRender())
		{
			m_ScreenElements.sort(SortBy_SharedPtr_Content<IScreenElement>());

			for(ScreenElementList::iterator i=m_ScreenElements.begin(); i!=m_ScreenElements.end(); ++i)
			{
				if ( (*i)->VIsVisible() )
				{
					(*i)->VOnRender(fTime, fElapsedTime);
				}
			}

            VRenderText();

			// Let the console render.
			m_Console.Render();

			// record the last successful paint
			m_lastDraw = m_currTick;
		}
		
		g_pApp->m_Renderer->VPostRender();
    }
}


//
// HumanView::VOnRestore						- Chapter 10, page 275
//
HRESULT HumanView::VOnRestore()
{
	HRESULT hr = S_OK;
	for(ScreenElementList::iterator i=m_ScreenElements.begin(); i!=m_ScreenElements.end(); ++i)
	{
		V_RETURN ( (*i)->VOnRestore() );
	}

	return hr;
}


//
// HumanView::VOnLostDevice						- not described in the book
//
//    Recursively calls VOnLostDevice for everything attached to the HumanView. 

HRESULT HumanView::VOnLostDevice() 
{
	HRESULT hr;

	for(ScreenElementList::iterator i=m_ScreenElements.begin(); i!=m_ScreenElements.end(); ++i)
	{
		V_RETURN ( (*i)->VOnLostDevice() );
	}

	return S_OK;
}



//
// HumanView::InitAudio							- Chapter X, page Y
//
bool HumanView::InitAudio()
{
	if (!g_pAudio)
	{
		g_pAudio = GCC_NEW DirectSoundAudio();		// use this line for DirectSound
	}

	if (!g_pAudio)
		return false;

	if (!g_pAudio->VInitialize(g_pApp->GetHwnd()))
		return false;

	return true;
}



void HumanView::TogglePause(bool active)
{
	// Pause or resume audio	
	if ( active )
	{
		if (g_pAudio) 
			g_pAudio->VPauseAllSounds();
	}
	else
	{
		if (g_pAudio)
			g_pAudio->VResumeAllSounds();
	}
}



//
// HumanView::VOnMsgProc						- Chapter 10, page 279
//
LRESULT CALLBACK HumanView::VOnMsgProc( AppMsg msg )
{
	// Iterate through the screen layers first
	// In reverse order since we'll send input messages to the 
	// screen on top
	for(ScreenElementList::reverse_iterator i=m_ScreenElements.rbegin(); i!=m_ScreenElements.rend(); ++i)
	{
		if ( (*i)->VIsVisible() )
		{
			if ( (*i)->VOnMsgProc( msg ) )
			{
				return 1;
			}
		}
	}

	LRESULT result = 0;
	switch (msg.m_uMsg) 
	{
        case WM_KEYDOWN:
			if (m_Console.IsActive())
			{
                // Let the console eat this.
			}
			else if (m_KeyboardHandler)
            {
				result = m_KeyboardHandler->VOnKeyDown(static_cast<const BYTE>(msg.m_wParam));
            }
			break;
	
        case WM_KEYUP:
			if (m_Console.IsActive())
			{
				// Let the console eat this.
			}
			else if (m_KeyboardHandler)
				result = m_KeyboardHandler->VOnKeyUp(static_cast<const BYTE>(msg.m_wParam));
			break;

		case WM_MOUSEMOVE:
			if (m_PointerHandler)
				result = m_PointerHandler->VOnPointerMove(Point(LOWORD(msg.m_lParam), HIWORD(msg.m_lParam)), 1);
			break;

		case WM_LBUTTONDOWN:
			if (m_PointerHandler)
			{
				SetCapture(msg.m_hWnd);
				result = m_PointerHandler->VOnPointerButtonDown(Point(LOWORD(msg.m_lParam), HIWORD(msg.m_lParam)), 1, "PointerLeft");
			}	
			break;

		case WM_LBUTTONUP:
			if (m_PointerHandler)
			{
				SetCapture(NULL);
				result = m_PointerHandler->VOnPointerButtonUp(Point(LOWORD(msg.m_lParam), HIWORD(msg.m_lParam)), 1, "PointerLeft");
			}
			break;

		case WM_RBUTTONDOWN:
			if (m_PointerHandler)
			{
				SetCapture(msg.m_hWnd);
				result = m_PointerHandler->VOnPointerButtonDown(Point(LOWORD(msg.m_lParam), HIWORD(msg.m_lParam)), 1, "PointerRight");
			}
			break;

		case WM_RBUTTONUP:
			if (m_PointerHandler)
			{
				SetCapture(NULL);
				result = m_PointerHandler->VOnPointerButtonUp(Point(LOWORD(msg.m_lParam), HIWORD(msg.m_lParam)), 1, "PointerRight");
			}
			break;
		case WM_CHAR:
			if (m_Console.IsActive())
			{
				const unsigned int oemScan = int( msg.m_lParam & ( 0xff << 16 ) ) >> 16;
				m_Console.HandleKeyboardInput( msg.m_wParam, MapVirtualKey( oemScan, 1 ), true );
			}
			else
			{
				//See if it was the console key.
				if (('~'==msg.m_wParam) || ('`'==msg.m_wParam))
				{
					m_Console.SetActive(true);
				}
			}
			break;
		default:
			return 0;
	}

	return 0;
}

//
// HumanView::VOnRestore						- Chapter 10, page 277
//
void HumanView::VOnUpdate(const int deltaMilliseconds)
{
	m_pProcessManager->UpdateProcesses(deltaMilliseconds);

	m_Console.Update(deltaMilliseconds);

	// This section of code was added post-press. It runs through the screenlist
	// and calls VOnUpdate. Some screen elements need to update every frame, one 
	// example of this is a 3D scene attached to the human view.
	//
	for(ScreenElementList::iterator i=m_ScreenElements.begin(); i!=m_ScreenElements.end(); ++i)
	{
		(*i)->VOnUpdate(deltaMilliseconds);
	}
}

//
// HumanView::VPushElement						- Chapter 10, page 274
//
void HumanView::VPushElement(shared_ptr<IScreenElement> pElement)
{
	m_ScreenElements.push_front(pElement);
}

//
// HumanView::VPopElement						- Chapter 10, page 274
//
//   
//
void HumanView::VRemoveElement(shared_ptr<IScreenElement> pElement)
{
	m_ScreenElements.remove(pElement);
}

//
// HumanView::VSetCameraOffset					- not described in the book
//
//   Sets a camera offset, useful for making a 1st person or 3rd person game
//
void HumanView::VSetCameraOffset(const Vec4 & camOffset )
{
	GCC_ASSERT(m_pCamera);
	if (m_pCamera)
	{
		m_pCamera->SetCameraOffset( camOffset );
	}
}



//
// HumanView::RegisterAllDelegates, HumanView::RemoveAllDelegates		- not described in the book
//
//   Aggregates calls to manage event listeners for the HumanView class.
//
void HumanView::RegisterAllDelegates(void)
{
    IEventManager* pGlobalEventManager = IEventManager::Get();
//	pGlobalEventManager->VAddListener(MakeDelegate(this, &HumanView::GameStateDelegate), EvtData_Game_State::sk_EventType);
    pGlobalEventManager->VAddListener(MakeDelegate(this, &HumanView::PlaySoundDelegate), EvtData_PlaySound::sk_EventType);
}

void HumanView::RemoveAllDelegates(void)
{
    IEventManager* pGlobalEventManager = IEventManager::Get();
//	pGlobalEventManager->VRemoveListener(MakeDelegate(this, &HumanView::GameStateDelegate), EvtData_Game_State::sk_EventType);
    pGlobalEventManager->VRemoveListener(MakeDelegate(this, &HumanView::PlaySoundDelegate), EvtData_PlaySound::sk_EventType);
}

//
// HumanView::PlaySoundDelegate							- Chapter X, page Y
//
void HumanView::PlaySoundDelegate(IEventDataPtr pEventData)
{
    shared_ptr<EvtData_PlaySound> pCastEventData = static_pointer_cast<EvtData_PlaySound>(pEventData);

    // play the sound a bullet makes when it hits a teapot
    Resource resource(pCastEventData->GetResource().c_str());
    shared_ptr<ResHandle> srh = static_pointer_cast<ResHandle>(g_pApp->m_ResCache->GetHandle(&resource));
    shared_ptr<SoundProcess> sfx(GCC_NEW SoundProcess(srh, 100, false));
    m_pProcessManager->AttachProcess(sfx);
}

//
// HumanView::GameStateDelegate							- Chapter X, page Y
//
void HumanView::GameStateDelegate(IEventDataPtr pEventData)
{
//    shared_ptr<EvtData_Game_State> pCastEventData = static_pointer_cast<EvtData_Game_State>(pEventData);
//    m_BaseGameState = pCastEventData->GetGameState(); 
}



//==============================================================
// HumanView::Console - defines the a class to manage a console to type in commands
//
//   Not described in the book. 
//
const int kCursorBlinkTimeMS = 500;

char const * const kExitString = "exit";
char const * const kClearString = "clear";

HumanView::Console::Console()
: m_bActive( false )
, m_bExecuteStringOnUpdate( false )
{
	m_ConsoleInputSize = 48;

	m_CurrentInputString = std::string("");

	m_InputColor = D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );
	m_OutputColor = D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

	m_CursorBlinkTimer = kCursorBlinkTimeMS;
	m_bCursorOn = true;
}

HumanView::Console::~Console()
{
}

void HumanView::Console::AddDisplayText( const std::string & newText )
{
	m_CurrentOutputString += newText;
	m_CurrentOutputString += '\n';
}

void HumanView::Console::SetDisplayText( const std::string & newText )
{
	m_CurrentOutputString = newText;
}

void HumanView::Console::Update( const int deltaMilliseconds )
{
	//Don't do anything if not active.
	if ( !m_bActive )
	{
		return;	//Bail!
	}

	//Do we have a string to execute?
	if (m_bExecuteStringOnUpdate)
	{
		const std::string renderedInputString = std::string(">") + m_CurrentInputString;
		if ( 0 == m_CurrentInputString.compare( kExitString ) )
		{
			SetActive( false );
			m_CurrentInputString.clear();
		}
		else if ( 0 == m_CurrentInputString.compare( kClearString ) )
		{
			m_CurrentOutputString = renderedInputString;	//clear
			m_CurrentInputString.clear();
		}
		else
		{
			//Put the input string into the output window.
			AddDisplayText( renderedInputString );

			const int retVal = true;

			//Attempt to execute the current input string...
            if (!m_CurrentInputString.empty())
			    LuaStateManager::Get()->VExecuteString(m_CurrentInputString.c_str());

			//Clear the input string
			m_CurrentInputString.clear();
		}

		//We're accepting input again.
		m_bExecuteStringOnUpdate = false;
        SetActive(false);
	}

	//Update the cursor blink timer...
	m_CursorBlinkTimer -= deltaMilliseconds;

	if ( m_CursorBlinkTimer < 0 )
	{
		m_CursorBlinkTimer = 0;

		m_bCursorOn = !m_bCursorOn;

		m_CursorBlinkTimer = kCursorBlinkTimeMS;
	}
}

void HumanView::Console::Render( )
{
	//Don't do anything if not active.
	if ( !m_bActive )
	{
		return;	//Bail!
	}

	D3DRenderer::g_pTextHelper->Begin();
	const D3DXCOLOR white( 1.0f, 1.0f, 1.0f, 1.0f );
	const D3DXCOLOR black( 0.0f, 0.0f, 0.0f, 1.0f );
	RECT inputTextRect, outputTextRect, shadowRect;

	//Display the console text at screen top, below the other text displayed.
	const std::string finalInputString = std::string( ">" ) + m_CurrentInputString + ( m_bCursorOn ? '\xa0' : '_' );
	inputTextRect.left = 10;
	inputTextRect.right = g_pApp->GetScreenSize().x - 10;
	inputTextRect.top = 100;
	inputTextRect.bottom = g_pApp->GetScreenSize().y - 10;

	const int kNumWideChars = 4096;
	WCHAR wideBuffer[ kNumWideChars ];
	AnsiToWideCch( wideBuffer, finalInputString.c_str(), kNumWideChars );

	D3DRenderer::g_pTextHelper->DrawTextLine( inputTextRect, DT_LEFT | DT_TOP | DT_CALCRECT, wideBuffer );

	//Draw with shadow first.
	shadowRect = inputTextRect;
	++shadowRect.left;
	++shadowRect.top;
	D3DRenderer::g_pTextHelper->SetForegroundColor( black );
	D3DRenderer::g_pTextHelper->DrawTextLine( shadowRect, DT_LEFT | DT_TOP, wideBuffer );

	//Now bright text.
	D3DRenderer::g_pTextHelper->SetForegroundColor( white );
	D3DRenderer::g_pTextHelper->DrawTextLine( inputTextRect, DT_LEFT | DT_TOP, wideBuffer );

	//Now display the output text just below the input text.
	outputTextRect.left = inputTextRect.left + 15;
	outputTextRect.top = inputTextRect.bottom + 15;
	outputTextRect.right = g_pApp->GetScreenSize().x - 10;
	outputTextRect.bottom = g_pApp->GetScreenSize().y - 10;
	AnsiToWideCch( wideBuffer, m_CurrentOutputString.c_str(), kNumWideChars );

	//Draw with shadow first.
	shadowRect = outputTextRect;
	++shadowRect.left;
	++shadowRect.top;
	D3DRenderer::g_pTextHelper->SetForegroundColor( black );
	D3DRenderer::g_pTextHelper->DrawTextLine( shadowRect, DT_LEFT | DT_TOP, wideBuffer );

	//Now bright text.
	D3DRenderer::g_pTextHelper->SetForegroundColor( white );
	D3DRenderer::g_pTextHelper->DrawTextLine( outputTextRect, DT_LEFT | DT_TOP, wideBuffer );

	D3DRenderer::g_pTextHelper->End();
}

void HumanView::Console::HandleKeyboardInput( const unsigned int keyVal, const unsigned int oemKeyVal, const bool bKeyDown )
{
	if ( true == m_bExecuteStringOnUpdate )
	{
		//We've already got a string to execute; ignore.
		return;
	}

	//See if it's a valid key press that we care about.
	switch( oemKeyVal )
	{
	    case VK_BACK:
		{
			const size_t strSize = m_CurrentInputString.size();
			if ( strSize > 0 )
			{
				m_CurrentInputString.erase((strSize - 1), 1);
			}
            break;
		}

    	case VK_RETURN:
		{
			m_bExecuteStringOnUpdate = true;	//Execute this string.
            break;
		}

    	default:
        {
		    m_CurrentInputString += (char)keyVal;
		    break;
        }
	}
}
