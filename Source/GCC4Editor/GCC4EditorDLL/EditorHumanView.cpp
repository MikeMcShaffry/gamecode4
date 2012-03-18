//========================================================================
// EditorHumanView.cpp : Editor's Human View
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


#include "GCC4EditorStd.h"

#include "..\..\GCC4\GameCode4\GameCode.h"
#include "..\..\GCC4\Actors\Actor.h"
#include "..\..\GCC4\Actors\RenderComponent.h"
#include "..\..\GCC4\Audio\Audio.h"
#include "..\..\GCC4\Audio\SoundProcess.h"
#include "..\..\GCC4\Graphics3D\SceneNodes.h"
#include "..\..\GCC4\Graphics3D\Geometry.h"
#include "..\..\GCC4\Graphics3D\Sky.h"
#include "..\..\GCC4\Graphics3D\MovementController.h"
#include "..\..\GCC4\EventManager\Events.h"
#include "..\..\GCC4\EventManager\EventManagerImpl.h"
#include "..\..\GCC4\Physics\PhysicsEventListener.h"

#include "EditorHumanView.h"
#include "Editor.h"


extern EditorApp globalApp;



//========================================================================
//
// EditorHumanView implementation
//
//========================================================================

//
// EditorHumanView::EditorHumanView				- Chapter 22, page 752
//
EditorHumanView::EditorHumanView(shared_ptr<IRenderer> renderer) 
	: HumanView(renderer)
{
	// The EditorHumanView differs from its counterpart in TeapotWars primarily
	// in the fact that we don't have any UI classes. Any level information
	// should be displayed in the C# app.
}

//
// EditorHumanView::VOnUpdate					- Chapter 22, page 752
//
void EditorHumanView::VOnUpdate( unsigned long deltaMilliseconds )
{
	// Much like TeapotWarsView::VOnUpdate, except
	// we only have one controller in the editor

	HumanView::VOnUpdate( deltaMilliseconds );

	if (m_pFreeCameraController)
	{
		m_pFreeCameraController->OnUpdate(deltaMilliseconds);
	}
}

//
// EditorHumanView::VLoadGameDelegate			- Chapter 22, page 752
//
bool EditorHumanView::VLoadGameDelegate(TiXmlElement* pLevelData)
{
	if (!HumanView::VLoadGameDelegate(pLevelData))
		return false;

	// We also make sure that the MovementController is hooked up
	// to the keyboard and mouse handlers, since this is our primary method
	// for moving the camera around.
	m_pFreeCameraController.reset(GCC_NEW MovementController(m_pCamera, 90, 0, true));
	m_pCamera->ClearTarget();

	m_KeyboardHandler = m_pFreeCameraController;
	m_PointerHandler = m_pFreeCameraController;

    // [rez] This needs to be here for you get a black screen
    m_pScene->VOnRestore();
    return true;
}




