//========================================================================
// TeapotEvents.cpp : defines game-specific events for TeapotWars
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
#include "TeapotEvents.h"
#include "../GCC4/LUAScripting/LuaStateManager.h"


// Note: these are game specific events, which really should be in TeapotEvents.cpp.
// They aren't because of an annoying dependancy in Network.cpp, which creates events
// from a bitstream. Once we get "real" streaming code in we can move these back to TeapotEvents.cpp.
const EventType EvtData_Fire_Weapon::sk_EventType(0x1b15b6a7);
const EventType EvtData_StartThrust::sk_EventType(0x1d62d48c);
const EventType EvtData_EndThrust::sk_EventType(0xe60f88a4);
const EventType EvtData_StartSteer::sk_EventType(0xf0b5b4fd);
const EventType EvtData_EndSteer::sk_EventType(0x176645ef);
//const EventType EvtData_PlaySound::sk_EventType(0x3d8118ee);
const EventType EvtData_Gameplay_UI_Update::sk_EventType(0x1002ded2);
const EventType EvtData_Set_Controlled_Actor::sk_EventType(0xbe5e3388);
const EventType EvtData_ScriptEventTest_ToLua::sk_EventType(0x56cff2bc);
const EventType EvtData_ScriptEventTest_FromLua::sk_EventType(0x53fbab61);


bool EvtData_Gameplay_UI_Update::VBuildEventFromScript(void)
{
    if (m_eventData.IsString())
    {
        m_gameplayUiString = m_eventData.GetString();
        return true;
    }

    return false;
}

bool EvtData_Set_Controlled_Actor::VBuildEventFromScript(void)
{
    if (m_eventData.IsInteger())
    {
        m_id = m_eventData.GetInteger();
        return true;
    }

    return false;
}

void EvtData_ScriptEventTest_ToLua::VBuildEventData(void)
{
    m_eventData.AssignNumber(LuaStateManager::Get()->GetLuaState(), m_num);
}

bool EvtData_ScriptEventTest_FromLua::VBuildEventFromScript(void)
{
    if (m_eventData.IsInteger())
    {
        m_num = m_eventData.GetInteger();
        return true;
    }

    return false;
}

void EvtData_Fire_Weapon::VBuildEventData(void)
{
	m_eventData.AssignNewTable(LuaStateManager::Get()->GetLuaState());
	m_eventData.SetInteger("id", m_id);
	m_eventData.SetInteger("roofus", 0);
}


void RegisterTeapotScriptEvents(void)
{
    REGISTER_SCRIPT_EVENT(EvtData_Fire_Weapon, EvtData_Fire_Weapon::sk_EventType);
    REGISTER_SCRIPT_EVENT(EvtData_Gameplay_UI_Update, EvtData_Gameplay_UI_Update::sk_EventType);
    REGISTER_SCRIPT_EVENT(EvtData_Set_Controlled_Actor, EvtData_Set_Controlled_Actor::sk_EventType);

    REGISTER_SCRIPT_EVENT(EvtData_StartThrust, EvtData_StartThrust::sk_EventType);
    REGISTER_SCRIPT_EVENT(EvtData_EndThrust, EvtData_EndThrust::sk_EventType);
    REGISTER_SCRIPT_EVENT(EvtData_StartSteer, EvtData_StartSteer::sk_EventType);
    REGISTER_SCRIPT_EVENT(EvtData_EndSteer, EvtData_EndSteer::sk_EventType);

    // tests
    REGISTER_SCRIPT_EVENT(EvtData_ScriptEventTest_ToLua, EvtData_ScriptEventTest_ToLua::sk_EventType);
    REGISTER_SCRIPT_EVENT(EvtData_ScriptEventTest_FromLua, EvtData_ScriptEventTest_FromLua::sk_EventType);
}

