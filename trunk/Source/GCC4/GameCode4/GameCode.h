#pragma once
//========================================================================
// GameCode.h : Defines the entry point for the application.
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

#include "../Mainloop/Initialization.h"
#include "../GameCode4/BaseGameLogic.h"
#include "../Graphics3D/SceneNodes.h"
#include "../UserInterface/UserInterface.h"

class FontHandler;
class BaseUI;
class EventManager;
class LuaStateManager;
class BaseSocketManager;
class NetworkEventForwarder;
class HumanView;

//
// class GameCodeApp							- Chapter X, page Y
//
class GameCodeApp
{
protected:

	HINSTANCE m_hInstance;					// the module instance
	bool m_bWindowedMode;					// true if the app is windowed, false if fullscreen
	bool m_bIsRunning;						// true if everything is initialized and the game is in the main loop
	bool m_bQuitRequested;					// true if the app should run the exit sequence
	bool m_bQuitting;						// true if the app is running the exit sequence
	Rect m_rcDesktop;						// current desktop size - not necessarilly the client window size
	Point m_screenSize;					// game screen size
	int m_iColorDepth;						// current color depth (16 or 32)
	bool m_bIsEditorRunning;				// true if the game editor is running


public:
	const Point &GetScreenSize()  { return m_screenSize; }

protected:
	std::map<std::wstring,std::wstring> m_textResource;
	std::map<std::wstring,UINT> m_hotkeys;

	int m_HasModalDialog;					// determines if a modal dialog is up
	int PumpUntilMessage (UINT msgEnd, WPARAM* pWParam, LPARAM* pLParam);
	int	EatSpecificMessages( UINT msgType, optional<LPARAM> lParam, optional<WPARAM> wParam);
	void FlashWhileMinimized();

public:

	GameCodeApp();

	// Game Application Data
	// You must define these in an inherited
	// class - see TeapotWarsApp for an example
	virtual TCHAR *VGetGameTitle()=0;
	virtual TCHAR *VGetGameAppDirectory()=0;
	virtual HICON VGetIcon()=0;

	// Win32 Specific Stuff
	HWND GetHwnd();
	HINSTANCE GetInstance() { return m_hInstance; }
	virtual bool InitInstance(HINSTANCE hInstance, LPWSTR lpCmdLine, HWND hWnd = NULL, int screenWidth = SCREEN_WIDTH, int screenHeight = SCREEN_HEIGHT);	

	static LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void *pUserContext );
	bool HasModalDialog() { return m_HasModalDialog!=0; }
	void ForceModalExit() { PostMessage(GetHwnd(), g_MsgEndModal, 0, g_QuitNoPrompt);	}

	LRESULT OnDisplayChange(int colorDepth, int width, int height);
	LRESULT OnPowerBroadcast(int event);
	LRESULT OnSysCommand(WPARAM wParam, LPARAM lParam);
	LRESULT OnClose();

	// Game Application actions
	LRESULT OnAltEnter();
	LRESULT OnNcCreate(LPCREATESTRUCT cs);

	bool LoadStrings(std::string language);
	std::wstring GetString(std::wstring sID);				
	int GetHotKeyForString(std::wstring sID);
	UINT MapCharToKeycode(const char pHotkey);

	int Modal(shared_ptr<IScreenElement> pModalScreen, int defaultAnswer);

	// FUTURE WORK - Seems like this class could be refactored, especailly the renderer !

	enum Renderer
	{
		Renderer_Unknown,
		Renderer_D3D9,
		Renderer_D3D11
	};

	shared_ptr<IRenderer> m_Renderer;

	static Renderer GetRendererImpl();


	// DirectX9 Specific Stuff
	static bool CALLBACK IsD3D9DeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
	static HRESULT CALLBACK OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
	static HRESULT CALLBACK OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
	static void CALLBACK OnD3D9FrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
	static void CALLBACK OnD3D9LostDevice( void* pUserContext );
	static void CALLBACK OnD3D9DestroyDevice( void* pUserContext );

	// DirectX 11 Specific Stuff
	static bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo, DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
	static HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
	static HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
	static void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext );
	static void CALLBACK OnD3D11DestroyDevice( void* pUserContext );
	static void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext );

	static bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );
	static void CALLBACK OnUpdateGame( double fTime, float fElapsedTime, void *pUserContext );


	// GameCode Specific Stuff
	BaseGameLogic *m_pGame;
	struct GameOptions m_Options;

	// You must define these functions to initialize your game.
	// Look for them in TeapotWars.h and TeapotWars.cpp
	virtual BaseGameLogic *VCreateGameAndView()=0;
	virtual bool VLoadGame(void);
	HumanView* GetHumanView();				// Added post press - it was convenient to grab the HumanView attached to the game.

	// File and Resource System
	class ResCache *m_ResCache;
	TCHAR m_saveGameDirectory[MAX_PATH];

	bool IsEditorRunning() { return m_bIsEditorRunning; }

	// Event manager
	EventManager *m_pEventManager;

	// Socket manager - could be server or client
	BaseSocketManager *m_pBaseSocketManager;
    NetworkEventForwarder* m_pNetworkEventForwarder;
	bool AttachAsClient();

protected:
    virtual void VCreateNetworkEventForwarder(void);
    virtual void VDestroyNetworkEventForwarder(void);

public:

	// Main loop processing
	void AbortGame() { m_bQuitting = true; }
	int GetExitCode() { return DXUTGetExitCode(); }
	bool IsRunning() { return m_bIsRunning; }
	void SetQuitting(bool quitting) { m_bQuitting = quitting; }

    BaseGameLogic* GetGameLogic(void) const { return m_pGame; }

protected:
    virtual void VRegisterGameEvents(void) {}

private:
    void RegisterEngineEvents(void);
};



extern GameCodeApp *g_pApp;
