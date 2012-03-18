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
#include "LuaStateManager.h"
#include "../Utilities/String.h"

#pragma comment(lib, "luaplus51-1201.lib")

LuaStateManager* LuaStateManager::s_pSingleton = NULL;

bool LuaStateManager::Create(void)
{
    if (s_pSingleton)
    {
        GCC_ERROR("Overwriting LuaStateManager singleton");
        SAFE_DELETE(s_pSingleton);
    }

    s_pSingleton = GCC_NEW LuaStateManager;
    if (s_pSingleton)
        return s_pSingleton->VInit();

    return false;
}

void LuaStateManager::Destroy(void)
{
    GCC_ASSERT(s_pSingleton);
    SAFE_DELETE(s_pSingleton);
}

LuaStateManager::LuaStateManager(void)
{
    m_pLuaState = NULL;
}

LuaStateManager::~LuaStateManager(void)
{
    if (m_pLuaState)
    {
        LuaPlus::LuaState::Destroy(m_pLuaState);
        m_pLuaState = NULL;
    }
}

bool LuaStateManager::VInit(void)
{
    m_pLuaState = LuaPlus::LuaState::Create(true);
    if (m_pLuaState == NULL)
        return false;

    // register functions
    m_pLuaState->GetGlobals().RegisterDirect("ExecuteFile", (*this), &LuaStateManager::VExecuteFile);
    m_pLuaState->GetGlobals().RegisterDirect("ExecuteString", (*this), &LuaStateManager::VExecuteString);

    return true;
}

void LuaStateManager::VExecuteFile(const char* path)
{
    int result = m_pLuaState->DoFile(path);
    if (result != 0)
        SetError(result);
}

void LuaStateManager::VExecuteString(const char* chunk)
{
    int result = 0;

    // Most strings are passed straight through to the Lua interpreter
    if (strlen(chunk) <= 1 || chunk[0] != '=')
    {
        result = m_pLuaState->DoString(chunk);
        if (result != 0)
            SetError(result);
    }

    // If the string starts with '=', wrap the statement in the print() function.
    else
    {
        std::string buffer("print(");
        buffer += (chunk + 1);
        buffer += ")";
        result = m_pLuaState->DoString(buffer.c_str());
        if (result != 0)
            SetError(result);
    }
}

void LuaStateManager::SetError(int errorNum)
{
    // Note: If we get an error, we're hosed because LuaPlus throws an exception.  So if this function
    // is called and the error at the bottom triggers, you might as well pack it in.

    LuaPlus::LuaStackObject stackObj(m_pLuaState,-1);
    const char* errStr = stackObj.GetString();
    if (errStr)
    {
        m_lastError = errStr;
        ClearStack();
    }
    else
        m_lastError = "Unknown Lua parse error";

    GCC_ERROR(m_lastError);
}

void LuaStateManager::ClearStack(void)
{
    m_pLuaState->SetTop(0);
}

LuaPlus::LuaObject LuaStateManager::GetGlobalVars(void)
{
    GCC_ASSERT(m_pLuaState);
    return m_pLuaState->GetGlobals();
}

LuaPlus::LuaState* LuaStateManager::GetLuaState(void) const
{
    return m_pLuaState;
}

LuaPlus::LuaObject LuaStateManager::CreatePath(const char* pathString, bool toIgnoreLastElement)
{
    StringVec splitPath;
    Split(pathString, splitPath, '.');
    if (toIgnoreLastElement)
        splitPath.pop_back();

    LuaPlus::LuaObject context = GetGlobalVars();
    for (auto it = splitPath.begin(); it != splitPath.end(); ++it)
    {
        // make sure we still have a valid context
        if (context.IsNil())
        {
            GCC_ERROR("Something broke in CreatePath(); bailing out (element == " + (*it) + ")");
            return context;  // this will be nil
        }

        // grab whatever exists for this element
        const std::string& element = (*it);
        LuaPlus::LuaObject curr = context.GetByName(element.c_str());

        if (!curr.IsTable())
        {
            // if the element is not a table and not nil, we clobber it
            if (!curr.IsNil())
            {
                GCC_WARNING("Overwriting element '" + element + "' in table");
                context.SetNil(element.c_str());
            }

            // element is either nil or was clobbered so add the new table
            context.CreateTable(element.c_str());
        }

        context = context.GetByName(element.c_str());
    }

    // if we get here, we have created the path
    return context;
}

void LuaStateManager::ConvertVec3ToTable(const Vec3& vec, LuaPlus::LuaObject& outLuaTable) const
{
    outLuaTable.AssignNewTable(GetLuaState());
    outLuaTable.SetNumber("x", vec.x);
    outLuaTable.SetNumber("y", vec.y);
    outLuaTable.SetNumber("z", vec.z);
}


void LuaStateManager::ConvertTableToVec3(const LuaPlus::LuaObject& luaTable, Vec3& outVec3) const
{
    LuaPlus::LuaObject temp;

    // x
    temp = luaTable.Get("x");
    if (temp.IsNumber())
        outVec3.x = temp.GetFloat();
    else
        GCC_ERROR("luaTable.x is not a number");

    // y
    temp = luaTable.Get("y");
    if (temp.IsNumber())
        outVec3.y = temp.GetFloat();
    else
        GCC_ERROR("luaTable.y is not a number");

    // z
    temp = luaTable.Get("z");
    if (temp.IsNumber())
        outVec3.z = temp.GetFloat();
    else
        GCC_ERROR("luaTable.z is not a number");
}


