// TeapotController.cpp - Controller class for the player teapots
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
#include "../GCC4/Graphics3D/SceneNodes.h"
#include "../GCC4/EventManager/EventManager.h"
#include "../GCC4/Physics/PhysicsEventListener.h"

#include "TeapotController.h"
#include "TeapotEvents.h"
#include "TeapotWars.h"

const float ACTOR_ACCELERATION = 6.5f * 8.0f;
const float ACTOR_ANGULAR_ACCELERATION = 22.0f;


////////////////////////////////////////////////////
// TeapotController Implementation
////////////////////////////////////////////////////


//
// TeapotController::TeapotController			- Chapter 19, page 735
//
TeapotController::TeapotController(shared_ptr<SceneNode> object)
: m_object(object)
{
	memset(m_bKey, 0x00, sizeof(m_bKey));
}



//
// TeapotController::VOnLButtonDown				- Chapter 19, page 735
//
bool TeapotController::VOnPointerButtonDown(const Point &mousePos, const int radius, const std::string &buttonName)
{
	if (buttonName != "PointerLeft")
		return false;
	ActorId actorId = m_object->VGet()->ActorId();
	GCC_ASSERT(actorId != INVALID_ACTOR_ID && _T("The teapot controller isn't attached to a valid actor!"));
    shared_ptr<EvtData_Fire_Weapon> pFireEvent(GCC_NEW EvtData_Fire_Weapon(actorId));
    IEventManager::Get()->VQueueEvent(pFireEvent);
	return true;
}



//
// TeapotController::OnUpdate				- Chapter 19, page 736
//
void TeapotController::OnUpdate(DWORD const deltaMilliseconds)  
{
    //
}

bool TeapotController::VOnKeyDown(const BYTE c)
{
    // update the key table
    m_bKey[c] = true;

    // send a thrust event if necessary
    if (c == 'W' || c == 'S')
    {
        const ActorId actorId = m_object->VGet()->ActorId();
        shared_ptr<EvtData_StartThrust> pEvent(GCC_NEW EvtData_StartThrust(actorId, (c == 'W' ? ACTOR_ACCELERATION : (-ACTOR_ACCELERATION))));
        IEventManager::Get()->VQueueEvent(pEvent);
    }

    // send a steer event if necessary
    if (c == 'A' || c == 'D')
    {
        const ActorId actorId = m_object->VGet()->ActorId();
        shared_ptr<EvtData_StartSteer> pEvent(GCC_NEW EvtData_StartSteer(actorId, (c == 'D' ? ACTOR_ANGULAR_ACCELERATION : (-ACTOR_ANGULAR_ACCELERATION))));
        IEventManager::Get()->VQueueEvent(pEvent);
    }

    return true;
}

bool TeapotController::VOnKeyUp(const BYTE c)
{
    // update the key table
    m_bKey[c] = false;

    // send an end thrust event if necessary
    if (c == 'W' || c == 'S')
    {
        const ActorId actorId = m_object->VGet()->ActorId();
        shared_ptr<EvtData_EndThrust> pEvent(GCC_NEW EvtData_EndThrust(actorId));
        IEventManager::Get()->VQueueEvent(pEvent);
    }

    // send an end steer event if necessary
    if (c == 'A' || c == 'D')
    {
        const ActorId actorId = m_object->VGet()->ActorId();
        shared_ptr<EvtData_EndSteer> pEvent(GCC_NEW EvtData_EndSteer(actorId));
        IEventManager::Get()->VQueueEvent(pEvent);
    }

    return true;
}
