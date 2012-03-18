#pragma once
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

#include "LuaPlus.h"


//---------------------------------------------------------------------------------------------------------------------
// LuaStateManager                              - Chapter 12, page 367
//---------------------------------------------------------------------------------------------------------------------
class LuaStateManager : public IScriptManager
{
    static LuaStateManager* s_pSingleton;
    LuaPlus::LuaState* m_pLuaState;
    std::string m_lastError;

public:
    // Singleton functions
    static bool Create(void);
    static void Destroy(void);
    static LuaStateManager* Get(void) { GCC_ASSERT(s_pSingleton); return s_pSingleton; }

    // IScriptManager interface
	virtual bool VInit(void) override;
    virtual void VExecuteFile(const char* resource) override;
    virtual void VExecuteString(const char* str) override;

    LuaPlus::LuaObject GetGlobalVars(void);
    LuaPlus::LuaState* GetLuaState(void) const;

    // public helpers
    LuaPlus::LuaObject CreatePath(const char* pathString, bool toIgnoreLastElement = false);
    void ConvertVec3ToTable(const Vec3& vec, LuaPlus::LuaObject& outLuaTable) const;
    void ConvertTableToVec3(const LuaPlus::LuaObject& luaTable, Vec3& outVec3) const;

private:
    void SetError(int errorNum);
    void ClearStack(void);

    // private constructor & destructor; call the static Create() and Destroy() functions instead
    explicit LuaStateManager(void);
    virtual ~LuaStateManager(void);
};

