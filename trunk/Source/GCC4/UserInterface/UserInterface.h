#pragma once
//========================================================================
// UserInterface.h : Defines UI elements of the GameCode4 application
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


#include "../Graphics3D/Scene.h"

const DWORD g_QuitNoPrompt = MAKELPARAM(-1,-1);
const UINT g_MsgEndModal = (WM_USER+100);

enum
{
	// Lower numbers get drawn first
	ZORDER_BACKGROUND, 
	ZORDER_LAYER1, 
	ZORDER_LAYER2, 
	ZORDER_LAYER3, 
	ZORDER_TOOLTIP
};


enum MessageBox_Questions {
	QUESTION_WHERES_THE_CD,
	QUESTION_QUIT_GAME,
};

class FontHandler;
class BaseUI;

// This class is a group of user interface controls.
// It can be modal or modeless.

//
// class BaseUI									- Chapter 10, page 286  
//
// This was factored to create a common class that
// implements some of the IScreenElement class common
// to modal/modeless dialogs
//
class BaseUI : public IScreenElement
{
protected:
	int					m_PosX, m_PosY;
	int					m_Width, m_Height;
	optional<int>		m_Result;
	bool				m_bIsVisible;
public:
	BaseUI() { m_bIsVisible = true; m_PosX = m_PosY = 0; m_Width = 100; m_Height = 100; }
	virtual void VOnUpdate(int) { };
	virtual HRESULT VOnLostDevice() { return S_OK; }
	virtual bool VIsVisible() const { return m_bIsVisible; }
	virtual void VSetVisible(bool visible) { m_bIsVisible = visible; }
};


//
// class ScreenElementScene						- Chapter X, page Y
//
class ScreenElementScene : public IScreenElement, public Scene
{
public:
	ScreenElementScene(shared_ptr<IRenderer> renderer) : Scene(renderer) { }
    virtual ~ScreenElementScene(void)
    {
        GCC_WARNING("~ScreenElementScene()");
    }

	// IScreenElement Implementation
	virtual void VOnUpdate(int deltaMS) { OnUpdate(deltaMS); };
	virtual HRESULT VOnRestore() 
		{ OnRestore(); return S_OK; }
	virtual HRESULT VOnRender(double fTime, float fElapsedTime)
		{ OnRender(); return S_OK; }
	virtual HRESULT VOnLostDevice()
		{ OnLostDevice(); return S_OK; } 
	virtual int VGetZOrder() const { return 0; }
	virtual void VSetZOrder(int const zOrder) { }

	// Don't handle any messages
	virtual LRESULT CALLBACK VOnMsgProc( AppMsg msg ) { return 0; }

	virtual bool VIsVisible() const { return true; }
	virtual void VSetVisible(bool visible) { }
	virtual bool VAddChild(ActorId id, shared_ptr<ISceneNode> kid) { return Scene::AddChild(id, kid); }
};



