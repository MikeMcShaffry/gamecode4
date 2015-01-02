//========================================================================
// ScriptEvent.cpp
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
#include "ScriptEvent.h"
#include "LuaStateManager.h"


ScriptEvent::CreationFunctions ScriptEvent::s_creationFunctions;

//---------------------------------------------------------------------------------------------------------------------
// Returns the event data after building it (if necessary)
//---------------------------------------------------------------------------------------------------------------------
LuaPlus::LuaObject ScriptEvent::GetEventData(void)
{
	if (!m_eventDataIsValid)
	{
		VBuildEventData();
		m_eventDataIsValid = true;
	}
	
	return m_eventData;
}


//---------------------------------------------------------------------------------------------------------------------
// This function is called when an event is sent from the script.  It sets the m_eventData member and calls 
// VBuildEventFromScript().
//---------------------------------------------------------------------------------------------------------------------
bool ScriptEvent::SetEventData(LuaPlus::LuaObject eventData)
{
	m_eventData = eventData;
	m_eventDataIsValid = VBuildEventFromScript();
	return m_eventDataIsValid;
}


//---------------------------------------------------------------------------------------------------------------------
// This function is called to register an event type with the script to link them.  The simplest way to do this is 
// with the REGISTER_SCRIPT_EVENT() macro, which calls this function.
//---------------------------------------------------------------------------------------------------------------------
void ScriptEvent::RegisterEventTypeWithScript(const char* key, EventType type)
{
	// get or create the EventType table
	LuaPlus::LuaObject eventTypeTable = LuaStateManager::Get()->GetGlobalVars().GetByName("EventType");
	if (eventTypeTable.IsNil())
		eventTypeTable = LuaStateManager::Get()->GetGlobalVars().CreateTable("EventType");

	// error checking
	GCC_ASSERT(eventTypeTable.IsTable());
	GCC_ASSERT(eventTypeTable[key].IsNil());
	
	// add the entry
	eventTypeTable.SetInteger(key, type);
}


//---------------------------------------------------------------------------------------------------------------------
// This function is called to map an event creation function pointer with the event type.  This allows an event to be
// created by only knowing its type.  This is required to allow scripts to trigger the instantiation and queueing of 
// events.
//---------------------------------------------------------------------------------------------------------------------
void ScriptEvent::AddCreationFunction(EventType type, CreateEventForScriptFunctionType pCreationFunctionPtr)
{
	GCC_ASSERT(s_creationFunctions.find(type) == s_creationFunctions.end());
	GCC_ASSERT(pCreationFunctionPtr != NULL);
	s_creationFunctions.insert(std::make_pair(type, pCreationFunctionPtr));
}

//---------------------------------------------------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------------------------------------------------
ScriptEvent* ScriptEvent::CreateEventFromScript(EventType type)
{
	CreationFunctions::iterator findIt = s_creationFunctions.find(type);
	if (findIt != s_creationFunctions.end())
	{
		CreateEventForScriptFunctionType func = findIt->second;
		return func();
	}
	else
	{
		GCC_ERROR("Couldn't find event type");
		return NULL;
	}
}


//---------------------------------------------------------------------------------------------------------------------
// Default implementation for VBuildEventData() sets the event data to nil.
//---------------------------------------------------------------------------------------------------------------------
void ScriptEvent::VBuildEventData(void)
{
	m_eventData.AssignNil(LuaStateManager::Get()->GetLuaState());
}
