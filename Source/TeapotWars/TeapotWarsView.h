#pragma once
//========================================================================
// TeapotWarsView.h : source file for the sample game
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


#include <DXUTGui.h>

#include "../GCC4/Graphics3D/SceneNodes.h"
#include "../GCC4/EventManager/EventManager.h"
#include "../GCC4/UserInterface/HumanView.h"


class MainMenuUI : public BaseUI
{
protected:
	CDXUTDialog m_SampleUI;					// dialog for sample specific controls
	void Set();
	bool m_bCreatingGame;
	int m_NumAIs;
	int m_NumPlayers;
	std::vector<std::wstring> m_Levels;
	int m_LevelIndex;
	std::string m_HostName;
	std::string m_HostListenPort;
	std::string m_ClientAttachPort;

public:
	MainMenuUI();
	virtual ~MainMenuUI();

	// IScreenElement Implementation
	virtual HRESULT VOnRestore();
	virtual HRESULT VOnRender(double fTime, float fElapsedTime);
	virtual int VGetZOrder() const { return 1; }
	virtual void VSetZOrder(int const zOrder) { }

	virtual LRESULT CALLBACK VOnMsgProc( AppMsg msg );
	static void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void *pUserContext );
	void CALLBACK _OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void *pUserContext );
};


class StandardHUD : public BaseUI
{
protected:
	CDXUTDialog m_HUD;                  // dialog for standard controls

public:
	StandardHUD();
	virtual ~StandardHUD();

	// IScreenElement Implementation
	virtual HRESULT VOnRestore();
	virtual HRESULT VOnRender(double fTime, float fElapsedTime);
	virtual int VGetZOrder() const { return 1; }
	virtual void VSetZOrder(int const zOrder) { }

	virtual LRESULT CALLBACK VOnMsgProc( AppMsg msg );
	static void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void *pUserContext );
};

class IEventManager;
class SoundProcess;
class TeapotController;

class MainMenuView : public HumanView
{
protected:
	shared_ptr<MainMenuUI> m_MainMenuUI; 
public:

	MainMenuView(); 
	~MainMenuView(); 
	virtual LRESULT CALLBACK VOnMsgProc( AppMsg msg );
	virtual void VRenderText();	
	virtual void VOnUpdate(unsigned long deltaMs);
};


class TeapotWarsHumanView : public HumanView
{
protected:
	bool  m_bShowUI;					// If true, it renders the UI control text
    std::wstring m_gameplayText;

	shared_ptr<TeapotController> m_pTeapotController;
	shared_ptr<MovementController> m_pFreeCameraController;
	shared_ptr<SceneNode> m_pTeapot;
	shared_ptr<StandardHUD> m_StandardHUD; 

public:
	TeapotWarsHumanView(shared_ptr<IRenderer> renderer); 
	virtual ~TeapotWarsHumanView();

	virtual LRESULT CALLBACK VOnMsgProc( AppMsg msg );	
	virtual void VRenderText();	
	virtual void VOnUpdate(unsigned long deltaMs);
	virtual void VOnAttach(GameViewId vid, ActorId aid);

	virtual void VSetControlledActor(ActorId actorId);
	virtual bool VLoadGameDelegate(TiXmlElement* pLevelData) override;

    // event delegates
    void GameplayUiUpdateDelegate(IEventDataPtr pEventData);
    void SetControlledActorDelegate(IEventDataPtr pEventData);

private:
    void RegisterAllDelegates(void);
    void RemoveAllDelegates(void);
};


class AITeapotView : public IGameView 
{
	friend class AITeapotViewListener;

private:
	shared_ptr<PathingGraph> m_pPathingGraph;

protected:
	GameViewId	m_ViewId;
	ActorId m_PlayerActorId;

public:
	AITeapotView(shared_ptr<PathingGraph> pPathingGraph);
	virtual ~AITeapotView();

	virtual HRESULT VOnRestore() { return S_OK; }
	virtual void VOnRender(double fTime, float fElapsedTime) {}
	virtual HRESULT VOnLostDevice() { return S_OK; }
	virtual GameViewType VGetType() { return GameView_AI; }
	virtual GameViewId VGetId() const { return m_ViewId; }
	virtual void VOnAttach(GameViewId vid, ActorId actorId) { m_ViewId = vid; m_PlayerActorId = actorId; }
	virtual LRESULT CALLBACK VOnMsgProc( AppMsg msg ) {	return 0; }
	virtual void VOnUpdate(unsigned long deltaMs) {}
	
	shared_ptr<PathingGraph> GetPathingGraph(void) const { return m_pPathingGraph; }
};
