#pragma once
//========================================================================
// ScriptProcess.h
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


#include "../Mainloop/Process.h"
#include "LuaStateManager.h"


//---------------------------------------------------------------------------------------------------------------------
// ScriptProcess                                                   - Chapter 12, page 370
//---------------------------------------------------------------------------------------------------------------------
class ScriptProcess : public Process
{
	unsigned long m_frequency, m_time;
	LuaPlus::LuaObject m_scriptInitFunction, m_scriptUpdateFunction;
	LuaPlus::LuaObject m_scriptSuccessFunction, m_scriptFailFunction, m_scriptAbortFunction;
    LuaPlus::LuaObject m_self;

public:
    static void RegisterScriptClass(void);
	
protected:
    // Process interface
	virtual void VOnInit(void);
	virtual void VOnUpdate(unsigned long deltaMs);
	virtual void VOnSuccess(void);
	virtual void VOnFail(void);
	virtual void VOnAbort(void);
	
private:
    // private helpers
    static void RegisterScriptClassFunctions(LuaPlus::LuaObject& metaTableObj);
    static LuaPlus::LuaObject CreateFromScript(LuaPlus::LuaObject self, LuaPlus::LuaObject constructionData, LuaPlus::LuaObject originalSubClass);
    virtual bool BuildCppDataFromScript(LuaPlus::LuaObject scriptClass, LuaPlus::LuaObject constructionData);

    // These are needed because the base-class version of these functions are all const and LuaPlus can't deal
    // with registering const functions.
    bool ScriptIsAlive(void) { return IsAlive(); }
    bool ScriptIsDead(void) { return IsDead(); }
    bool ScriptIsPaused(void) { return IsPaused(); }

    // This wrapper function is needed so we can translate a Lua script object to something C++ can use.
    void ScriptAttachChild(LuaPlus::LuaObject child);

    // don't allow construction outside of this class
    explicit ScriptProcess(void);
};

