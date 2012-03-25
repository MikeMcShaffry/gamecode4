//========================================================================
// Editor.cpp : source file for the editor application and logic in C++
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

#include "resource.h"
#include "Editor.h"

#include "../../GCC4/Actors/Actor.h"
#include "../../GCC4/Actors/TransformComponent.h"
#include "../../GCC4/AI/Pathing.h"
#include "../../GCC4/EventManager/Events.h"
#include "../../GCC4/EventManager/EventManagerImpl.h"
#include "../../GCC4/GameCode4/GameCode.h"
#include "../../GCC4/Physics/Physics.h"
#include "../../GCC4/Physics/PhysicsEventListener.h"
#include "../../GCC4/MainLoop/Initialization.h"


#include "../../GCC4/Utilities/String.h"

#include <direct.h>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "advapi32.lib")			// useful for Windows Registry queries
#pragma comment(lib, "OleAut32.lib")			// needed for the C# / C++ BSTR conversions

#pragma comment(lib, "dxut.lib")
#pragma comment(lib, "dxutopt.lib")


#if defined(_M_IX86)
	#if defined(_DEBUG)
		#pragma comment(lib, "bulletcollision_debug.lib")
		#pragma comment(lib, "bulletdynamics_debug.lib")
		#pragma comment(lib, "linearmath_debug.lib")
	#else
		#pragma comment(lib, "bulletcollision.lib")
		#pragma comment(lib, "bulletdynamics.lib")
		#pragma comment(lib, "linearmath.lib")
	#endif
#elif defined(_M_X64)
	#if defined(_DEBUG)
		#pragma comment(lib, "bulletcollision_x64__debug.lib")
		#pragma comment(lib, "bulletdynamics_x64__debug.lib")
		#pragma comment(lib, "linearmath_x64_debug.lib")
	#else
		#pragma comment(lib, "bulletcollision_x64.lib")
		#pragma comment(lib, "bulletdynamics_x64.lib")
		#pragma comment(lib, "linearmath_x64.lib")
	#endif
#else
	#error Preprocessor defines can't figure out which Bullet library to use.
#endif



#pragma comment(lib, "zlibstat.lib")

#pragma comment(lib, "libogg_static.lib")
#pragma comment(lib, "libvorbis_static.lib")
#pragma comment(lib, "libvorbisfile_static.lib")

#pragma comment(lib, "tinyxml.lib")




EditorApp globalApp;

//========================================================================
//
// Editor Implementation
//
//========================================================================

//
// EditorApp::VCreateGameAndView						- Chapter 22, page 749
//
BaseGameLogic* EditorApp::VCreateGameAndView()
{
	BaseGameLogic *game = NULL;

	game = GCC_NEW EditorLogic();
    game->Init();

	shared_ptr<IGameView> gameView(GCC_NEW EditorHumanView(g_pApp->m_Renderer));
	game->VAddView(gameView);

	return game;
}

HICON EditorApp::VGetIcon()
{
	return LoadIcon(GetInstance(), MAKEINTRESOURCE(IDI_ICON1));
}


//========================================================================
//
// EditorLogic Implementation
//
//========================================================================

//
// EditorLogic::EditorLogic					- Chapter 22, page 750
//
EditorLogic::EditorLogic()
: BaseGameLogic()
{
	// FUTURE WORK - this is a little hardcoded - but since the editor is built along with the 
	//    framework it is probably ok to assume a current working directory.
	m_ProjectDirectory = getcwd(NULL, 0);
	int slashGamePos = m_ProjectDirectory.rfind("\\Game");
	m_ProjectDirectory = m_ProjectDirectory.substr(0, slashGamePos);

	// We're not using physics for the editor, but as an optional feature,
	// you can hook up a control that will allow you to toggle the physics
	// on and off. I leave this as a future assignment!
	m_pPhysics.reset(CreateNullPhysics());
}


//
// EditorLogic::~EditorLogic				- Chapter 22, not in the book
//
EditorLogic::~EditorLogic()
{

}

//
// EditorLogic::VLoadGame					- Chapter 20, page 751
//
bool EditorLogic::VLoadGame(const char* levelName)
{
	while (m_actors.size() > 0)
	{
		ActorId id = m_actors.begin()->first;
		VDestroyActor(id);
	}

	if (!BaseGameLogic::VLoadGame(levelName))
	{
		return false;	
	}

	VChangeState(BGS_Running);
	return true;
}

//
// EditorLogic::GetHumanView					- Chapter 20, page 751
//
shared_ptr<EditorHumanView> EditorLogic::GetHumanView() 
{
	GCC_ASSERT(m_gameViews.size()==1);
	shared_ptr<IGameView> pGameView = *m_gameViews.begin();
	shared_ptr<EditorHumanView> editorHumanView = static_pointer_cast<EditorHumanView>( pGameView );
	return editorHumanView; 
}
