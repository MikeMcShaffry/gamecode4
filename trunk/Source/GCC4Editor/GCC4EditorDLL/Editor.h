#pragma once
//========================================================================
// Editor.h : source file for the editor application and logic in C++
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


//========================================================================
//  Content References in Game Coding Complete 4th Edition
// 
//  class Editor						- Chapter 22, page 749
//  class EditorLogic					- Chapter 22, page 750
//

#include "..\..\GCC4\GameCode4\GameCode.h"

#include "..\..\GCC4\Physics\Physics.h"
#include "..\..\GCC4\Physics\PhysicsEventListener.h"
#include "..\..\GCC4\MainLoop\Initialization.h"

#include "..\..\GCC4\EventManager\Events.h"
#include "..\..\GCC4\EventManager\EventManagerImpl.h"

#include "EditorHumanView.h"

using namespace std;

class EditorLogic;

class EditorApp : public GameCodeApp
{
public:
	EditorApp() : GameCodeApp() { m_bIsEditorRunning = true; }
	TCHAR *VGetGameTitle() { return _T("GameCode4 Editor"); }
	TCHAR *VGetGameAppDirectory() { return _T("Game Coding Complete 4\\Editor\\1.0"); }
	HICON VGetIcon();

protected:
	BaseGameLogic *VCreateGameAndView();
};


class EditorLogic : public BaseGameLogic
{
public:
	EditorLogic();
	~EditorLogic();
	
	virtual bool VLoadGame(const char* levelName);
	const std::string &GetProjectDirectory(void) { return m_ProjectDirectory; }

	// We need to expose this information so that the C# app can
	// know how big of an array to allocate to hold the list of
	// actors
	int GetNumActors() { return (int)m_actors.size(); }

	// Exposes the actor map so that the global functions
	// can retrieve actor information
	const ActorMap& GetActorMap() { return m_actors; }

	bool IsRunning() { return (BGS_Running == m_State); }
	shared_ptr<EditorHumanView> GetHumanView();

protected:
	std::string m_ProjectDirectory;
};

