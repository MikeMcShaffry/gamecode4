//========================================================================
// ScriptProcess.cpp
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
#include "ScriptProcess.h"


const char* SCRIPT_PROCESS_NAME = "ScriptProcess";


ScriptProcess::ScriptProcess(void)
{
    LuaPlus::LuaState* pLuaState = LuaStateManager::Get()->GetLuaState();

	m_frequency = 0;
	m_time = 0;
	m_scriptInitFunction.AssignNil(pLuaState);
	m_scriptUpdateFunction.AssignNil(pLuaState);
	m_scriptSuccessFunction.AssignNil(pLuaState);
	m_scriptFailFunction.AssignNil(pLuaState);
	m_scriptAbortFunction.AssignNil(pLuaState);
}

bool ScriptProcess::BuildCppDataFromScript(LuaPlus::LuaObject scriptClass, LuaPlus::LuaObject constructionData)
{
	if (scriptClass.IsTable())
	{
		// OnInit()
		LuaPlus::LuaObject temp = scriptClass.GetByName("OnInit");
		if (temp.IsFunction())
			m_scriptInitFunction = temp;
		
		// OnUpdate()
		temp = scriptClass.GetByName("OnUpdate");
		if (temp.IsFunction())
		{
			m_scriptUpdateFunction = temp;
		}
		else
		{
			GCC_ERROR("No OnUpdate() found in script process; type == " + std::string(temp.TypeName()));
			return false;  // we're hosed, though I think returning false here just crashes the game :-/
		}
		
		// OnSuccess()
		temp = scriptClass.GetByName("OnSuccess");
		if (temp.IsFunction())
			m_scriptSuccessFunction = temp;

		// OnFail()
		temp = scriptClass.GetByName("OnFail");
		if (temp.IsFunction())
			m_scriptFailFunction = temp;

		// OnAbort()
		temp = scriptClass.GetByName("OnAbort");
		if (temp.IsFunction())
			m_scriptAbortFunction = temp;
	}
	else
	{
	    GCC_ERROR("scriptClass is not a table in ScriptProcess::BuildCppDataFromScript()");
	    return false;
	}
	
	if (constructionData.IsTable())
	{

        for (LuaPlus::LuaTableIterator constructionDataIt(constructionData); constructionDataIt; constructionDataIt.Next())
        {
            const char* key = constructionDataIt.GetKey().GetString();
            LuaPlus::LuaObject val = constructionDataIt.GetValue();

            if (strcmp(key, "frequency") == 0 && val.IsInteger())
                m_frequency = val.GetInteger();
            else
                m_self.SetObject(key, val);
        }
	}

	return true;
}

void ScriptProcess::VOnInit(void)
{
	Process::VOnInit();
	if (!m_scriptInitFunction.IsNil())
	{
		LuaPlus::LuaFunction<void> func(m_scriptInitFunction);
		func(m_self);
	}
	
	// No update function so bail immediately.  We may want to consider calling Succeed() here so that the child 
	// process is attached normally, but right now I'm assuming that the reason no OnUpdate() function was found
	// is due to a bug on the script side.  That means the child process may be dependent on this one.
	if (!m_scriptUpdateFunction.IsFunction())
		Fail();
}

void ScriptProcess::VOnUpdate(unsigned long deltaMs)
{
	m_time += deltaMs;
	if (m_time >= m_frequency)
	{
		LuaPlus::LuaFunction<void> func(m_scriptUpdateFunction);
		func(m_self, m_time);
		m_time = 0;
	}
}

void ScriptProcess::VOnSuccess(void)
{
	if (!m_scriptSuccessFunction.IsNil())
	{
		LuaPlus::LuaFunction<void> func(m_scriptSuccessFunction);
		func(m_self);
	}
}

void ScriptProcess::VOnFail(void)
{
	if (!m_scriptFailFunction.IsNil())
	{
		LuaPlus::LuaFunction<void> func(m_scriptFailFunction);
		func(m_self);
	}
}

void ScriptProcess::VOnAbort(void)
{
	if (!m_scriptAbortFunction.IsNil())
	{
		LuaPlus::LuaFunction<void> func(m_scriptAbortFunction);
		func(m_self);
	}
}

void ScriptProcess::ScriptAttachChild(LuaPlus::LuaObject child)
{
    if (child.IsTable())
    {
        LuaPlus::LuaObject obj = child.GetByName("__object");
        if (!obj.IsNil())
        {
            // Casting a raw ptr to a smart ptr is generally bad, but Lua has no concept of what a shared_ptr 
            // is.  There's no easy way around it.
            shared_ptr<Process> pProcess(static_cast<Process*>(obj.GetLightUserData()));
            GCC_ASSERT(pProcess);
            AttachChild(pProcess);
        }
        else
        {
            GCC_ERROR("Attempting to attach child to ScriptProcess with no valid __object");
        }
    }
    else
    {
        GCC_ERROR("Invalid object type passed into ScriptProcess::ScriptAttachChild(); type = " + std::string(child.TypeName()));
    }
}

void ScriptProcess::RegisterScriptClass(void)
{
	LuaPlus::LuaObject metaTableObj = LuaStateManager::Get()->GetGlobalVars().CreateTable(SCRIPT_PROCESS_NAME);
	metaTableObj.SetObject("__index", metaTableObj);
	metaTableObj.SetObject("base", metaTableObj);  // base refers to the parent class; ie the metatable
	metaTableObj.SetBoolean("cpp", true);
	RegisterScriptClassFunctions(metaTableObj);
    metaTableObj.RegisterDirect("Create", &ScriptProcess::CreateFromScript);
}

LuaPlus::LuaObject ScriptProcess::CreateFromScript(LuaPlus::LuaObject self, LuaPlus::LuaObject constructionData, LuaPlus::LuaObject originalSubClass)
{
	// Note: The self parameter is not use in this function, but it allows us to be consistent when calling
	// Create().  The Lua version of this function needs self.
	GCC_LOG("Script", std::string("Creating instance of ") + SCRIPT_PROCESS_NAME);
	ScriptProcess* pObj = GCC_NEW ScriptProcess;
	
    pObj->m_self.AssignNewTable(LuaStateManager::Get()->GetLuaState());
	if (pObj->BuildCppDataFromScript(originalSubClass, constructionData))
	{
		LuaPlus::LuaObject metaTableObj = LuaStateManager::Get()->GetGlobalVars().Lookup(SCRIPT_PROCESS_NAME);
		GCC_ASSERT(!metaTableObj.IsNil());
		
		pObj->m_self.SetLightUserData("__object", pObj);
		pObj->m_self.SetMetaTable(metaTableObj);
	}
	else
	{
		pObj->m_self.AssignNil(LuaStateManager::Get()->GetLuaState());
		SAFE_DELETE(pObj);
	}
	
	return pObj->m_self;
}

void ScriptProcess::RegisterScriptClassFunctions(LuaPlus::LuaObject& metaTableObj)
{
	metaTableObj.RegisterObjectDirect("Succeed",    (Process*)0, &Process::Succeed);
    metaTableObj.RegisterObjectDirect("Fail",       (Process*)0, &Process::Fail);
    metaTableObj.RegisterObjectDirect("Pause",      (Process*)0, &Process::Pause);
    metaTableObj.RegisterObjectDirect("UnPause",    (Process*)0, &Process::UnPause);
    metaTableObj.RegisterObjectDirect("IsAlive",    (ScriptProcess*)0, &ScriptProcess::ScriptIsAlive);
    metaTableObj.RegisterObjectDirect("IsDead",     (ScriptProcess*)0, &ScriptProcess::ScriptIsDead);
    metaTableObj.RegisterObjectDirect("IsPaused",   (ScriptProcess*)0, &ScriptProcess::ScriptIsPaused);
    metaTableObj.RegisterObjectDirect("AttachChild",(ScriptProcess*)0, &ScriptProcess::ScriptAttachChild);
}
